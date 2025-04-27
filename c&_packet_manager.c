#include "c&_packet_manager.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println, c_ampersand_error_with_message için
#include "c&_stdlib.h"
// C& G/Ç operasyonları (c_ampersand_io_*, c_ampersand_file_handle, C_AMPERSAND_O_* bayrakları, c_ampersand_io_unlink) için
#include "c&_io_operations.h"
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"
// Sahne64 API başlık dosyası (sahne_error_t için)
#include "sahne.h"

// stdlib.h, stdio.h kaldırıldı
// #include <stdlib.h>
// #include <stdio.h>

#include <string.h> // strlen, snprintf, memcpy için
#include <stdbool.h> // bool için

// --- Dahili Sabitler ve Veri Yapıları ---

// Kurulu paketlerin bilgi dosyalarının bulunduğu dizin (Sahne64 dosya sisteminde)
// Gerçek bir sistemde bu yol konfigüre edilebilir olmalıdır.
static const char *INSTALLED_PACKAGES_INFO_DIR = "/opt/sahne64/packages/";
static const char *PACKAGE_INFO_FILE_EXTENSION = ".info";

// Path stringini ve tahsis edilen boyutunu tutan yardımcı yapı
typedef struct {
    char *ptr;
    size_t size; // c_ampersand_free için gerekli olan tahsis boyutu
} path_info_t;

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
// static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);

// Paket yöneticisine özgü hata kodları (varsayım: c&_stdlib.h'de tanımlı değilse)
#ifndef C_AMPERSAND_ERROR_PACKAGE_ALREADY_INSTALLED
#define C_AMPERSAND_ERROR_PACKAGE_ALREADY_INSTALLED (C_AMPERSAND_ERROR_CUSTOM_START + 1) // Veya uygun bir değer
#endif
#ifndef C_AMPERSAND_ERROR_PACKAGE_NOT_INSTALLED
#define C_AMPERSAND_ERROR_PACKAGE_NOT_INSTALLED (C_AMPERSAND_ERROR_CUSTOM_START + 2) // Veya uygun bir değer
#endif
// ... diğer PM hata kodları (DOWNLOAD_FAILED, VALIDATION_FAILED, EXTRACTION_FAILED vb.)


// --- Yardımcı Fonksiyonlar ---

// Belirli bir paket için bilgi dosyasının tam dosya yolunu oluşturur.
// Yol stringini C& belleğinde tahsis eder ve pointer+boyut pair'i olarak döndürür.
// Hata olursa {NULL, 0} döner.
static path_info_t get_package_info_path(const char *package_name) {
    path_info_t result = {NULL, 0};

    if (package_name == NULL || strlen(package_name) == 0) {
        // Geçersiz paket adı
        c_ampersand_println("PM HATA: Bilgi dosyası yolu oluşturulurken geçersiz paket adı.");
        return result; // {NULL, 0}
    }

    size_t dir_len = strlen(INSTALLED_PACKAGES_INFO_DIR);
    size_t name_len = strlen(package_name);
    size_t ext_len = strlen(PACKAGE_INFO_FILE_EXTENSION);
    // Dizin + Paket Adı + Uzantı + null sonlandırıcı
    size_t total_len = dir_len + name_len + ext_len + 1;

    char *path_ptr = NULL;
    // Yol stringi için C& belleği tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(total_len, (void**)&path_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği hatası c_ampersand_allocate içinde loglanır/raporlanır
        return result; // {NULL, 0}
    }

    // Tam yolu buffer'a formatla
    // Varsayım: snprintf Sahne64/C& stdlib'de mevcut.
    int chars_written = snprintf(path_ptr, total_len, "%s%s%s", INSTALLED_PACKAGES_INFO_DIR, package_name, PACKAGE_INFO_FILE_EXTENSION);

    // snprintf hatası veya taşma kontrolü
    if (chars_written < 0 || (size_t)chars_written >= total_len) {
        c_ampersand_println("PM HATA: Bilgi dosyası yolu formatlanırken hata veya buffer taşması.");
        c_ampersand_free(path_ptr, total_len); // Tahsis edilen belleği serbest bırak
        return result; // {NULL, 0}
    }

    result.ptr = path_ptr;
    result.size = total_len; // Tahsis edilen toplam boyutu kaydet (free için)

    return result; // Başarı
}

// Bir paketin kurulu olup olmadığını kontrol eder (basit bilgi dosyası varlığı kontrolü).
// get_package_info_path ve c_ampersand_io_open/close kullanır.
bool c_ampersand_pm_is_package_installed(const char *package_name) {
    // Parametre doğrulama
     if (package_name == NULL || strlen(package_name) == 0) {
        // Geçersiz paket adı, kurulu değil varsayalım
        return false;
    }

    // Bilgi dosyasının yolunu al
    path_info_t path_info = get_package_info_path(package_name);
    if (path_info.ptr == NULL) {
        // Yol oluşturma hatası (muhtemelen bellek). Kurulu değil varsayalım ve loglayalım.
        c_ampersand_println("PM UYARI: '%s' paket bilgi yolu oluşturulamadı (bellek?). Kurulu değil varsayılıyor.", package_name);
        return false;
    }

    // Bilgi dosyasını salt okunur modda açmayı dene. Başarılı olursa var demektir.
    c_ampersand_file_handle handle = C_AMPERSAND_INVALID_FILE_HANDLE;
    c_ampersand_result open_res = c_ampersand_io_open(path_info.ptr, C_AMPERSAND_O_RDONLY, &handle);

    // Yol stringi artık gerekmiyor, serbest bırak.
    c_ampersand_free(path_info.ptr, path_info.size); // Boyut get_package_info_path tarafından sağlanıyor.

    bool is_installed = false;
    if (open_res.code == C_AMPERSAND_OK) {
        // Dosya açıldı, yani kurulu. Handle'ı kapat.
        c_ampersand_io_close(handle); // Kapatma hatası burada görmezden gelinebilir.
        is_installed = true;
    } else if (open_res.code == C_AMPERSAND_ERROR_FILE_NOT_FOUND) {
        // Dosya bulunamadı hatası, paket kurulu değil.
        is_installed = false;
    } else {
        // Diğer I/O hataları (izin, vb.). Bu bir sorun, loglayalım ama kurulu değil varsayalım.
        // c_ampersand_result_print_error(open_res); // Detaylı hata mesajını yazdır
        c_ampersand_println("PM UYARI: '%s' paket bilgi dosyası kontrol edilirken I/O hatası (%d). Kurulu değil varsayılıyor.", package_name, open_res.code);
        // c_ampersand_result_free(&open_res); // Eğer hata mesajı allocate edildiyse serbest bırak
        is_installed = false;
    }

    return is_installed;
}

// --- Paket Yöneticisi Fonksiyonlarının Implementasyonu ---

// Bir paketi verilen isim ve versiyonla kurar.
// Kurulum simülasyonu: Bilgi dosyası oluşturur.
c_ampersand_result c_ampersand_pm_install_package(const char *package_name, const char *version) {
    // Parametre doğrulama
    if (package_name == NULL || strlen(package_name) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Paketin zaten kurulu olup olmadığını kontrol et
    if (c_ampersand_pm_is_package_installed(package_name)) {
        // Paket zaten kurulu
        // c_ampersand_println("PM BILGI: Paket zaten kurulu: %s", package_name); // pm_is_package_installed içinde loglandı.
        return (c_ampersand_result){C_AMPERSAND_ERROR_PACKAGE_ALREADY_INSTALLED, NULL, 0}; // Belirli bir hata kodu dönelim
    }

    c_ampersand_println("PM BILGI: Paket kuruluyor: %s (versiyon: %s)", package_name, version ? version : "son sürüm");

    // --- Gerçek Kurulum Mantığı Taslağı ---
    // 1. Paket deposundan paketi indir (Ağ API'si gerektirir). Hata olursa dön.
    // 2. İndirilen paket dosyasını doğrula (Checksum, İmza, Format - c&_package_format API'si gerekebilir). Hata olursa dön.
    // 3. Paketi sistemdeki uygun bir konuma çıkar (c&_package_format API'si ve Dizin/Dosya I/O API'ları gerektirir). Hata olursa dön.
    // 4. Paket info dizini yoksa oluştur (Dizin API'si).
    // 5. Kurulu paketler bilgi dizinine bir bilgi dosyası yaz (c_ampersand_io_open/write/close). Paketin adı, versiyonu, kurulu olduğu yer, bağımlılıkları gibi bilgileri kaydet.
    // ... diğer kurulum sonrası adımlar (sembolik linkler, cache güncelleme vb.)

    // --- Bilgi Dosyası Oluşturma Simülasyonu ---
    // Kurulu paketi belirtmek için sadece info dosyası oluşturuyoruz.

    path_info_t install_path_info = get_package_info_path(package_name);
    if (install_path_info.ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, c_ampersand_error_with_message(C_AMPERSAND_ERROR_OUT_OF_MEMORY, "Kurulum bilgi dosyası yolu için bellek yetersiz.").message, 0}; // Mesajlı OOM
    }

    // Bilgi dosyasını yazma modunda oluştur/aç
    c_ampersand_file_handle handle = C_AMPERSAND_INVALID_FILE_HANDLE;
    // C_AMPERSAND_O_WRONLY | C_AMPERSAND_O_CREAT | C_AMPERSAND_O_TRUNC = Yazma, Oluştur (yoksa), İçeriği Sil (varsa)
    c_ampersand_result open_res = c_ampersand_io_open(install_path_info.ptr, C_AMPERSAND_O_WRONLY | C_AMPERSAND_O_CREAT | C_AMPERSAND_O_TRUNC, &handle);

    // Yol stringi artık gerekmiyor
    c_ampersand_free(install_path_info.ptr, install_path_info.size);

    if (open_res.code != C_AMPERSAND_OK) {
        // Dosya açma/oluşturma hatası
        // c_ampersand_io_open zaten detaylı hata mesajı içerebilir
        return open_res; // I/O hatasını ilet
    }

    // Versiyon bilgisini formatla (stack buffer)
    char version_info_buffer[64];
    const char *pkg_version = version ? version : "unknown";
    int chars_written = snprintf(version_info_buffer, sizeof(version_info_buffer), "version=%s\n", pkg_version);

    // snprintf hatası veya taşma kontrolü
    if (chars_written < 0 || (size_t)chars_written >= sizeof(version_info_buffer)) {
         c_ampersand_println("PM HATA: Versiyon bilgisi formatlanırken hata veya buffer taşması.");
         c_ampersand_io_close(handle); // Dosyayı kapat
         // Oluşturulan bilgi dosyasını silmek de iyi olabilir, ama zorunlu değil.
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_GENERIC, "Kurulum bilgi dosyası formatlama hatası.");
    }

    // Versiyon bilgisini dosyaya yaz
    size_t bytes_written; // Yazılan byte sayısı c_ampersand_io_write tarafından doldurulur
    c_ampersand_result write_res = c_ampersand_io_write(handle, version_info_buffer, (size_t)chars_written, &bytes_written);

     // Dosyayı kapat
    c_ampersand_result close_res = c_ampersand_io_close(handle);

    // Yazma hatasını kontrol et (kapatma hatasından önce)
    if (write_res.code != C_AMPERSAND_OK) {
        return write_res; // Yazma hatasını ilet
    }

    // Kapatma hatasını kontrol et
    if (close_res.code != C_AMPERSAND_OK) {
        return close_res; // Kapatma hatasını ilet
    }


    c_ampersand_println("PM BILGI: Paket başarıyla kuruldu (simülasyon): %s", package_name);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir paketi kaldırır.
// Kaldırma simülasyonu: Bilgi dosyasını siler.
c_ampersand_result c_ampersand_pm_uninstall_package(const char *package_name) {
    // Parametre doğrulama
    if (package_name == NULL || strlen(package_name) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Paketin kurulu olup olmadığını kontrol et
    if (!c_ampersand_pm_is_package_installed(package_name)) {
        // Paket kurulu değil
        c_ampersand_println("PM BILGI: Paket kurulu değil: %s", package_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_PACKAGE_NOT_INSTALLED, NULL, 0}; // Belirli bir hata kodu dönelim
    }

    c_ampersand_println("PM BILGI: Paket kaldırılıyor: %s", package_name);

    // --- Gerçek Kaldırma Mantığı Taslağı ---
    // 1. Kurulu paketler bilgi dosyasından paketin kurulu olduğu yeri ve dosyaların listesini oku (c_ampersand_io_open/read/close/parse).
    // 2. Pakete ait tüm dosyaları ve dizinleri sil (Dizin/Dosya API'ları, c_ampersand_io_unlink).
    // 3. Kurulu paketler bilgi dizinindeki bilgi dosyasını sil.

    // --- Bilgi Dosyası Silme Simülasyonu ---
    // Sadece info dosyasını siliyoruz.

    path_info_t uninstall_path_info = get_package_info_path(package_name);
    if (uninstall_path_info.ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, c_ampersand_error_with_message(C_AMPERSAND_ERROR_OUT_OF_MEMORY, "Kaldırma bilgi dosyası yolu için bellek yetersiz.").message, 0}; // Mesajlı OOM
    }

    // Bilgi dosyasını sil (unlink Sahne64 kaynak API'sini sarmalar)
    c_ampersand_result unlink_res = c_ampersand_io_unlink(uninstall_path_info.ptr);

    // Yol stringi artık gerekmiyor
    c_ampersand_free(uninstall_path_info.ptr, uninstall_path_info.size);

    // Silme hatasını kontrol et
    if (unlink_res.code != C_AMPERSAND_OK) {
        // Silme hatası (izin hatası, dosya kilitli vb.)
        return unlink_res; // I/O hatasını ilet
    }

    c_ampersand_println("PM BILGI: Paket başarıyla kaldırıldı (simülasyon): %s", package_name);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Kurulu paketleri listeler (simülasyon: hardcoded liste veya info dizinini oku).
c_ampersand_result c_ampersand_pm_list_installed_packages() {
    c_ampersand_println("PM BILGI: Kurulu paketler:");

    // --- Gerçek Listeleme Mantığı Taslağı ---
    // 1. Kurulu paketler bilgi dizinini aç (Sahne64 dizin okuma API'si veya C& sarmalayıcısı gereklidir).
    // 2. Dizindeki her ".info" dosyası için:
    //    a. Dosyayı aç (c_ampersand_io_open).
    //    b. İçindeki paket adı ve versiyon bilgisini oku (c_ampersand_io_read_line, parse).
    //    c. Bilgileri c_ampersand_println ile yazdır.
    //    d. Dosyayı kapat (c_ampersand_io_close).
    //    e. Pars edilen string belleklerini serbest bırak (c_ampersand_free).
    // 3. Dizin Handle'ını kapat (Dizin API'si).
    // 4. Tüm I/O, bellek ve parsing hatalarını uygun şekilde raporla.

    // --- Hardcoded/Simüle Liste ---
    // Şu an için sadece hardcoded veya basit dosya varlığına dayalı listeleme.
    c_ampersand_println("PM NOT: Gerçek listeleme implemente edilecek.");
    c_ampersand_println("- ornek_paket_1 (versiyon 1.0) [simülasyon]");
    c_ampersand_println("- baska_paket (versiyon 2.5) [simülasyon]");

    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// Paket depolarını günceller (simülasyon).
c_ampersand_result c_ampersand_pm_update_repositories() {
    c_ampersand_println("PM BILGI: Paket depoları güncelleniyor...");

    // --- Gerçek Güncelleme Mantığı Taslağı ---
    // 1. Yapılandırılmış depo URL'lerinden (sistem ayarı?) ağ üzerinden depo meta bilgilerini indir (Ağ API'si gerektirir).
    // 2. İndirilen meta bilgileri doğrula (örn. imza kontrolü).
    // 3. Yerel depo cache'ini güncelle (Dosya I/O API'ları).
    // 4. Ağ ve I/O hatalarını uygun şekilde raporla.

    // Şu an sadece bir bilgilendirme mesajı gösteriliyor.
    c_ampersand_println("PM NOT: Paket depoları güncelleme implementasyonu taslağı burada çalışacak.");
    c_ampersand_println("PM BILGI: Paket depoları güncellendi (simülasyon).");

    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

#include "c&_io.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle tipi, resource fonksiyonları ve MODE_* bayrakları için)
#include "sahne.h" // Varsayım: Sahne64 API'si Handle, u64, sahne_error_t, SAHNE_ERROR_*, MODE_* tanımlarını içerir.

#include <string.h> // strlen için
#include <stddef.h> // size_t, NULL için
#include <stdint.h> // uint32_t için


// Standart çıktı kaynağı için Handle (dışarıdan atanması gereken global değişken)
u64 c_ampersand_stdout_handle = 0; // 0 genellikle geçersiz Handle kabul edilir

// Helper function to map Sahne64 error codes to C& error codes
static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err) {
    // Varsayım: C_AMPERSAND_* hata kodları c&_stdlib.h veya c&_error_handling.h'de tanımlı
    switch (sahne_err) {
        case SAHNE_SUCCESS: return C_AMPERSAND_OK; // Başarı durumu (fonksiyon hata dönmeyecekse)
        case SAHNE_ERROR_OUT_OF_MEMORY: return C_AMPERSAND_ERROR_OUT_OF_MEMORY;
        case SAHNE_ERROR_INVALID_PARAMETER: return C_AMPERSAND_ERROR_INVALID_ARGUMENT;
        case SAHNE_ERROR_RESOURCE_NOT_FOUND: return C_AMPERSAND_ERROR_FILE_NOT_FOUND; // Dosya özelinde
        case SAHNE_ERROR_PERMISSION_DENIED: return C_AMPERSAND_ERROR_PERMISSION_DENIED;
        case SAHNE_ERROR_RESOURCE_BUSY: return C_AMPERSAND_ERROR_IO; // Daha spesifik bir IO hatası olabilir
        case SAHNE_ERROR_INTERRUPTED: return C_AMPERSAND_ERROR_IO; // Duruma göre farklı eşleme olabilir
        case SAHNE_ERROR_INVALID_OPERATION: return C_AMPERSAND_ERROR_IO; // Geçersiz okuma/yazma gibi
        case SAHNE_ERROR_NOT_SUPPORTED: return C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION; // Veya C_AMPERSAND_ERROR_IO
        case SAHNE_ERROR_INVALID_HANDLE: return C_AMPERSAND_ERROR_INVALID_ARGUMENT; // Geçersiz FD gibi
        // ... diğer SAHNE_ERROR_* kodları için eşlemeler ...
        default: return C_AMPERSAND_ERROR_IO; // Eşleşmeyen diğer Sahne hataları için genel IO hatası
    }
}


// Kaynak/dosya açar (Sahne64 resource_acquire'a karşılık gelir).
c_ampersand_result c_ampersand_file_open(const char *path, uint32_t flags, c_ampersand_file_descriptor *fd) {
    if (path == NULL || fd == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    u64 handle = 0; // SAHNE64 Handle'ı alacağımız değişken
    // Sahne64 resource_acquire fonksiyonu sahne_error_t döner ve handle'ı out_handle* parametresine yazar.
    // path'in Sahne64'ün Resource ID formatında olması gerekir (örn. "sahne://fs/usr/file.txt").
    // resource_acquire path için char* ve size_t alır, biz strlen kullanıyoruz.
    sahne_error_t sahne_err = sahne_resource_acquire(path, strlen(path), flags, &handle);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        *fd = 0; // Hata durumunda fd'yi geçersiz yap
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err)};
    }

    // Başarılı olursa döndürülen Handle'ı C& dosya tanımlayıcısına ata
    *fd = handle;
    // Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de tanımlı
    return C_AMPERSAND_OK_RESULT;
}

// Kaynaktan okur (Sahne64 resource_read'e karşılık gelir).
c_ampersand_result c_ampersand_file_read(c_ampersand_file_descriptor fd, void *buffer, size_t count, size_t *bytes_read) {
     // fd bir Sahne64 Handle'ı (u64)
    if (buffer == NULL || count == 0) {
         // Buffer NULL veya boyut 0 ise okuma yapmaya gerek yok (veya hata?)
         if (bytes_read != NULL) *bytes_read = 0;
         return C_AMPERSAND_OK_RESULT;
    }
    if (fd == 0) { // Geçersiz Handle (0 genellikle geçersizdir)
         if (bytes_read != NULL) *bytes_read = 0;
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz dosya tanımlayıcısı
    }
    // bytes_read NULL olabilir, kontrol etmek gerekir.
    if (bytes_read == NULL) {
         // Okunan byte sayısını döndürmek istenmiyor, ancak Sahne API'si gerektiriyorsa
         // geçici bir değişken kullanabiliriz veya API'nin NULL kabul ettiğini varsayabiliriz.
         // Sahne API'si out_bytes_read* aldığı için NULL KABUL ETMEZ varsayalım.
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // bytes_read NULL olamaz
    }


    size_t actual_bytes_read = 0;
    // Sahne64 resource_read fonksiyonu sahne_error_t döner ve okunan byte sayısını out_bytes_read* parametresine yazar.
    sahne_error_t sahne_err = sahne_resource_read(fd, buffer, count, &actual_bytes_read);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
         *bytes_read = 0; // Hata durumunda okunan byte sayısı 0
         return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err)};
    }

    // Başarılı olursa okunan byte sayısını çıktı parametresine yaz
    *bytes_read = actual_bytes_read;

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Kaynağa yazar (Sahne64 resource_write'a karşılık gelir).
c_ampersand_result c_ampersand_file_write(c_ampersand_file_descriptor fd, const void *buffer, size_t count, size_t *bytes_written) {
     // fd bir Sahne64 Handle'ı (u64)
     if (buffer == NULL || count == 0) {
         // Buffer NULL veya boyut 0 ise yazmaya gerek yok (veya hata?)
         if (bytes_written != NULL) *bytes_written = 0;
         return C_AMPERSAND_OK_RESULT;
     }
     if (fd == 0) { // Geçersiz Handle (0 genellikle geçersizdir)
         if (bytes_written != NULL) *bytes_written = 0;
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz dosya tanımlayıcısı
     }
     // bytes_written NULL olabilir mi? Sahne API'sine bağlı. Null kabul etmez varsayalım.
     if (bytes_written == NULL) {
          return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // bytes_written NULL olamaz
     }


    size_t actual_bytes_written = 0;
    // Sahne64 resource_write fonksiyonu sahne_error_t döner ve yazılan byte sayısını out_bytes_written* parametresine yazar.
    sahne_error_t sahne_err = sahne_resource_write(fd, buffer, count, &actual_bytes_written);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
         *bytes_written = 0; // Hata durumunda yazılan byte sayısı 0
         return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err)};
    }

    // Başarılı olursa yazılan byte sayısını çıktı parametresine yaz
    *bytes_written = actual_bytes_written;

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Kaynağı kapatır/serbest bırakır (Sahne64 resource_release'e karşılık gelir).
c_ampersand_result c_ampersand_file_close(c_ampersand_file_descriptor fd) {
    // fd bir Sahne64 Handle'ı (u64)
    if (fd == 0) { // Geçersiz Handle
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Zaten kapalı veya geçersiz
    }

    // Sahne64 resource_release fonksiyonu sahne_error_t döner.
    sahne_error_t sahne_err = sahne_resource_release(fd);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err)};
    }

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Standart çıktıya bir string yazdırır.
// c_ampersand_stdout_handle Handle'ını kullanır.
c_ampersand_result c_ampersand_print(const char *str) {
     if (str == NULL) {
         // NULL string yazdırmak bir hata olabilir veya sadece işlem yapmayabilir.
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Hata dönelim
     }
     // Standart çıktı Handle'ı geçerli mi kontrol et
     if (c_ampersand_stdout_handle == 0) {
         // stdout Handle'ı ayarlanmamış (runtime başlatma hatası?)
         // Bu durumda çıktı veremeyiz.
         // Varsayım: C_AMPERSAND_ERROR_IO c&_stdlib.h'de tanımlı
         return (c_ampersand_result){C_AMPERSAND_ERROR_IO}; // Veya daha spesifik bir hata kodu
     }

    size_t len = c_ampersand_string_length(str);
    size_t bytes_written = 0; // Yazılan byte sayısı çıktısı (c_ampersand_file_write gerektiriyor)

    // Standart çıktı Handle'ına yaz
    return c_ampersand_file_write(c_ampersand_stdout_handle, str, len, &bytes_written);
    // c_ampersand_file_write zaten hata durumunu c_ampersand_result olarak döndürüyor.
}

// Standart çıktıya bir string yazdırır ve satır sonu ekler.
c_ampersand_result c_ampersand_println(const char *str) {
     // Stringi yazdır
    c_ampersand_result result = c_ampersand_print(str);

    // String yazdırma başarılıysa satır sonu karakterini yazdır
    if (result.code == C_AMPERSAND_OK) {
        result = c_ampersand_print("\n"); // "\n" stringini yazdır
    }
    // String yazdırma başarılı değilse ilk hatayı döndür.
    // Satır sonu yazdırma başarılı değilse de ikinci hatayı döndür.
    return result;
}

// Basit bir string uzunluğu fonksiyonu (c&_string.c'de daha gelişmiş bir sürümü olabilir)
size_t c_ampersand_string_length(const char *str) {
    // NULL pointer kontrolü ekleyelim
     if (str == NULL) return 0;

    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

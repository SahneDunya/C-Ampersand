#include "c&_repository.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println, c_ampersand_error_with_message için
#include "c&_stdlib.h"
// Paket meta bilgi yapısı ve free fonksiyonu için
#include "c&_package_format.h" // c_ampersand_package_metadata tanımı ve c_ampersand_package_free_metadata
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"
// Varsayımsal ağ/G/Ç sarmalayıcıları için (depo indeksi indirme/okuma)
 #include "c&_network.h" // Eğer HTTP desteklenecekse
 #include "c&_io_operations.h" // Eğer yerel dosya deposu desteklenecekse

#include <string.h> // strlen, strcmp, memcpy için
#include <stdbool.h> // bool için


// --- Dahili Veri Yapıları ve Fonksiyonlar ---

// Yapılandırılmış depoların listesi (basit bir statik dizi olarak implemente edilmiştir)
#define MAX_REPOSITORIES 10
static c_ampersand_repository_info repositories[MAX_REPOSITORIES];
static int repository_count = 0;

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
 static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);


// Bir c_ampersand_repository_info yapısındaki heap'te ayrılmış string alanlarını serbest bırakır.
// c_ampersand_free kullanır.
static c_ampersand_result c_ampersand_pm_repository_free_info(c_ampersand_repository_info *info) {
    if (info == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL info yapısını serbest bırakmak güvenlidir
    }

    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT;

    // url stringini serbest bırak
    if (info->url != NULL && info->url_allocated_size > 0) {
        c_ampersand_result res = c_ampersand_free(info->url, info->url_allocated_size);
        if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
        info->url = NULL;
        info->url_allocated_size = 0;
    }

    // ... varsa diğer heap'te ayrılmış alanları da serbest bırak

    return final_result; // Serbest bırakma işlemlerinin sonucunu döndür
}


// Bir URL'in zaten eklenmiş olup olmadığını kontrol eder.
static bool is_repository_exists(const char *url) {
    if (url == NULL) return false; // Geçersiz URL

    for (int i = 0; i < repository_count; ++i) {
        // urls[i] geçerli mi kontrol et
        if (repositories[i].url != NULL && strcmp(repositories[i].url, url) == 0) {
            return true;
        }
    }
    return false;
}

// Bir depodan paket indeksini indirir/okur ve işler (parse eder).
// url: İndeks indirilecek/okunacak depo URL'i.
// Dönüş: Başarı veya hata (ağ/G/Ç hatası, parsing hatası, bellek hatası).
// İndeks verisini içsel bir yapıda saklar.
static c_ampersand_result fetch_and_parse_repository_index(const char *url) {
    if (url == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("REPO NOT: Depo indeksi güncelleniyor: %s", url);

    // --- Implementasyon Taslağı ---
    // 1. URL'in tipini belirle (HTTP, HTTPS, dosya:// vb.). Basit string kontrolü veya URL parsing.
    // 2. URL tipine göre indeksi indir veya oku:
    //   a. HTTP/HTTPS için: c_ampersand_network_http_get(url, &buffer) gibi bir Ağ API'sı kullan.
    //   b. Yerel dosya için: c_ampersand_io_open(path, C_AMPERSAND_O_RDONLY, &handle) ve c_ampersand_io_read/read_line gibi G/Ç API'sı kullan.
    // 3. İndirme/okuma sırasında oluşacak Ağ veya G/Ç hatalarını raporla (C_AMPERSAND_ERROR_INDEX_FETCH_FAILED).
    // 4. İndirilen/okunan indeksi (bellek buffer'ında olmalı) ayrıştır (parse).
    //   a. İndeks formatı metin tabanlıysa (örn. package_name:version:description satırları), string işleme fonksiyonları (strlen, strcmp, strchr, strstr, strtok vb.) veya özel bir parser kullan.
    //   b. İkili (binary) veya yapılandırılmış (JSON, YAML) bir format ise uygun bir parsing kütüphanesi kullan.
    // 5. Ayrıştırılan paket bilgilerini (adı, versiyonu, açıklaması, dosya adı vb.) içsel bir veri yapısında sakla. Bu yapı, get_package_metadata ve list_packages tarafından kullanılacak.
    //   (Bu içsel yapı, her depo için ayrı ayrı yönetilmelidir. c_ampersand_repository_info struct'ına bir alan eklenebilir - örn. void *index_data).
    //   (Eski indeks verisi varsa önce serbest bırakılmalıdır).
    // 6. Parsing sırasında oluşacak format hatalarını raporla (C_AMPERSAND_ERROR_INDEX_PARSE_FAILED) ve kısmen ayrıştırılmış veriyi temizle.
    // 7. İndirilen/okunan indeks buffer'ını serbest bırak (c_ampersand_free).
    // 8. Bellek yetersizliği (parsing veya içsel yapı için) hatalarını raporla (C_AMPERSAND_ERROR_OUT_OF_MEMORY).
    // ...

    // Örnek: Şu an sadece bir placeholder.
     return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Depo indeksi indirme/parsing henüz implemente edilmedi.");

    c_ampersand_println("REPO NOT: Depo indeksi indirme/parsing implementasyon taslağı burada çalışacak.");

    // Placeholder başarı dönüşü
    c_ampersand_println("REPO NOT: Depo indeksi güncellendi (simülasyon): %s", url);
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// --- Depo Yönetimi Fonksiyonlarının Implementasyonu ---

// Depo yönetimi modülünü başlatır (statik diziyi ve sayacı sıfırlar).
c_ampersand_result c_ampersand_pm_repository_init() {
    // Statik depo dizisini sıfırla
    memset(repositories, 0, sizeof(c_ampersand_repository_info) * MAX_REPOSITORIES);
    repository_count = 0; // Sayacı sıfırla

    // İsteğe bağlı: Başlangıçta varsayılan depoları ekle
     c_ampersand_pm_repository_add("http://sahne64.org/packages/main"); // Örnek
     c_ampersand_pm_repository_add("file:///opt/sahne64/localrepo"); // Örnek yerel

    c_ampersand_println("REPO BILGI: Depo yönetimi başlatıldı.");
    return C_AMPERSAND_OK_RESULT;
}

// Depo yönetimi modülünü kapatır (saklanan URL stringlerini serbest bırakır).
c_ampersand_result c_ampersand_pm_repository_shutdown() {
    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Saklanan tüm depo URL stringlerini serbest bırak
    for (int i = 0; i < repository_count; ++i) {
        // Depo bilgisinin (URL ve varsa indeks verisi) serbest bırakılması
        c_ampersand_result free_res = c_ampersand_pm_repository_free_info(&repositories[i]); // URL stringini serbest bırakır

        // İndeks verisi de serbest bırakılmalıdır, bu c_ampersand_pm_repository_free_info içinde yapılmalıdır.
        // Varsayım: c_ampersand_pm_repository_free_info artık index_data'yı da serbest bırakır.

        if (free_res.code != C_AMPERSAND_OK) {
            // Serbest bırakma hatası olursa logla ve hatayı kaydet
             c_ampersand_result_print_error(free_res);
            if (final_result.code == C_AMPERSAND_OK) final_result = free_res; // İlk hatayı sakla
             c_ampersand_result_free(&free_res); // Mesajı serbest bırak (eğer varsa)
        }
        // Dizideki elemanı sıfırla (güvenlik için)
        memset(&repositories[i], 0, sizeof(c_ampersand_repository_info));
    }
    repository_count = 0; // Sayacı sıfırla

    c_ampersand_println("REPO BILGI: Depo yönetimi kapatıldı.");
    return final_result; // Kapanış işleminin genel sonucunu dön
}


// Yeni bir paket deposu ekler. URL stringini kopyalar ve saklar.
c_ampersand_result c_ampersand_pm_repository_add(const char *url) {
    // Parametre doğrulama
    if (url == NULL || strlen(url) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Dizi limitini kontrol et
    if (repository_count >= MAX_REPOSITORIES) {
        c_ampersand_println("REPO UYARI: Maksimum depo limitine (%d) ulaşıldı. Depo eklenemedi: %s", MAX_REPOSITORIES, url);
        return (c_ampersand_result){C_AMPERSAND_ERROR_LIMIT_REACHED, c_ampersand_error_with_message(C_AMPERSAND_ERROR_LIMIT_REACHED, "Maksimum depo limitine (%d) ulaşıldı.", MAX_REPOSITORIES).message, 0}; // Limit aşıldı hatası
    }

    // Deponun zaten eklenmiş olup olmadığını kontrol et
    if (is_repository_exists(url)) {
        c_ampersand_println("REPO BILGI: Bu depo zaten eklenmiş: %s", url);
        return (c_ampersand_result){C_AMPERSAND_ERROR_REPOSITORY_ALREADY_ADDED, c_ampersand_error_with_message(C_AMPERSAND_ERROR_REPOSITORY_ALREADY_ADDED, "Depo zaten eklenmiş: %s", url).message, 0}; // Zaten eklenmiş hatası
    }

    // URL stringi için C& belleği tahsis et
    size_t url_len = strlen(url);
    size_t allocation_size = url_len + 1; // Null sonlandırıcı dahil
    char *url_ptr = NULL;
    c_ampersand_result alloc_res = c_ampersand_allocate(allocation_size, (void**)&url_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
         // Bellek yetersizliği
         return alloc_res; // Bellek hatasını ilet (mesajı allocate içinde gelir)
    }

    // Stringi kopyala
    memcpy(url_ptr, url, allocation_size);

    // Dizide pointer ve boyutu sakla
    // Yeni depo bilgisi yapısını sıfırla
    memset(&repositories[repository_count], 0, sizeof(c_ampersand_repository_info));
    repositories[repository_count].url = url_ptr;
    repositories[repository_count].url_allocated_size = allocation_size; // Tahsis edilen toplam boyutu kaydet

    repository_count++; // Sayacı artır

    c_ampersand_println("REPO BILGI: Depo eklendi: %s", url);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir paket deposunu kaldırır. URL stringini bellekten siler.
c_ampersand_result c_ampersand_pm_repository_remove(const char *url) {
    // Parametre doğrulama
    if (url == NULL || strlen(url) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Depoyu dizide ara
    for (int i = 0; i < repository_count; ++i) {
        // urls[i] geçerli mi kontrol et
        if (repositories[i].url != NULL && strcmp(repositories[i].url, url) == 0) {
            // Depo bulundu, URL stringini serbest bırak ve dizideki boşluğu doldur
            c_ampersand_result free_res = c_ampersand_pm_repository_free_info(&repositories[i]); // URL ve varsa indeks datasını serbest bırakır
            // Free hatası olsa bile kaldırma işlemine devam etmeye çalışalım.
            if (free_res.code != C_AMPERSAND_OK) {
                 c_ampersand_println("REPO UYARI: Kaldırılan depo '%s' bilgisi serbest bırakılırken hata: %d", url, free_res.code);
                  c_ampersand_result_print_error(free_res); // Detaylı hatayı yazdır
                  c_ampersand_result_free(&free_res); // Mesajı serbest bırak
            }

            // Dizideki boşluğu doldur (elemanları sola kaydır)
            for (int j = i; j < repository_count - 1; ++j) {
                repositories[j] = repositories[j + 1]; // Yapı kopyalama
            }
            // Son elemanı sıfırla (güvenlik için)
            memset(&repositories[repository_count - 1], 0, sizeof(c_ampersand_repository_info));

            repository_count--; // Sayacı azalt

            c_ampersand_println("REPO BILGI: Depo kaldırıldı: %s", url);
            return C_AMPERSAND_OK_RESULT; // Başarı
        }
    }

    // Depo bulunamadı
    c_ampersand_println("REPO UYARI: Depo bulunamadı: %s", url);
    return (c_ampersand_result){C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND, c_ampersand_error_with_message(C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND, "Depo bulunamadı: %s", url).message, 0}; // Bulunamadı hatası
}

// Tüm yapılandırılmış paket depolarının indekslerini günceller.
c_ampersand_result c_ampersand_pm_repository_update_all() {
    c_ampersand_println("REPO BILGI: Tüm depolar güncelleniyor...");
    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Statik dizideki depoları döngüye al
    for (int i = 0; i < repository_count; ++i) {
        // URL geçerli mi kontrol et
        if (repositories[i].url == NULL) continue;

        // Depo indeksini indir/oku ve ayrıştır
        c_ampersand_result update_result = fetch_and_parse_repository_index(repositories[i].url);

        if (update_result.code != C_AMPERSAND_OK) {
            // Güncelleme hatası olursa logla ve genel sonuca kaydet
            c_ampersand_println("REPO HATA: Depo güncelleme hatası: %s (Hata Kodu: %d)", repositories[i].url, update_result.code);
             c_ampersand_result_print_error(update_result); // Detaylı hatayı yazdır
            if (final_result.code == C_AMPERSAND_OK) final_result = update_result; // İlk hatayı sakla
             c_ampersand_result_free(&update_result); // Mesajı serbest bırak
            // Hata olsa bile diğer depoları güncellemeye devam et
        }
    }

    // Tüm depolar denendi. Genel sonucu dön.
    if (final_result.code == C_AMPERSAND_OK) {
        c_ampersand_println("REPO BILGI: Tüm depolar başarıyla güncellendi.");
    } else {
        c_ampersand_println("REPO UYARI: Tüm depolar güncellenirken hatalar oluştu.");
    }
    return final_result; // Başarı veya ilk hatayı dön
}

// Belirli bir paket deposunun indeksini günceller.
c_ampersand_result c_ampersand_pm_repository_update(const char *url) {
    // Parametre doğrulama
    if (url == NULL || strlen(url) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Deponun eklenmiş olup olmadığını kontrol et
    if (!is_repository_exists(url)) {
        c_ampersand_println("REPO UYARI: Depo bulunamadı: %s", url);
        return (c_ampersand_result){C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND, c_ampersand_error_with_message(C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND, "Depo bulunamadı: %s", url).message, 0}; // Bulunamadı hatası
    }

    // Depo indeksini indir/oku ve ayrıştır
    return fetch_and_parse_repository_index(url); // İşlemin sonucunu direkt ilet
}

// Bir depodan belirli bir paket için meta bilgi alır.
// Depo indeksinin daha önce güncellenmiş olması gerekir ve içsel yapıda saklı olmalıdır.
c_ampersand_result c_ampersand_pm_repository_get_package_metadata(const char *url, const char *package_name, c_ampersand_package_metadata *metadata) {
    // Parametre doğrulama
    if (url == NULL || package_name == NULL || metadata == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // metadata yapısını başlangıçta sıfırla (free edilebilir olması için)
    memset(metadata, 0, sizeof(c_ampersand_package_metadata));

    c_ampersand_println("REPO NOT: Paket meta bilgisi alınıyor: %s (%s)", package_name, url);

    // --- Implementasyon Taslağı ---
    // 1. url'e karşılık gelen depoyu içsel depo listesinde bul (is_repository_exists gibi). Depo bulunamazsa hata dön (C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND).
    // 2. Bulunan deponun içsel indeks verisini kontrol et. İndeks verisi henüz indirilmemiş/parse edilmemişse (örn. repositories[i].index_data == NULL), hata dön (örn. C_AMPERSAND_ERROR_INDEX_NOT_FETCHED).
    // 3. İçsel indeks verisi içinde package_name adına sahip paketi ara.
    //   (Bu arama, indeks veri yapısının tipine bağlıdır - dizi, bağlı liste, hash map vb.)
    // 4. Paket bulunursa, o paketin meta bilgilerini (adı, versiyonu, açıklaması vb.) metadata yapısına kopyala.
    //   a. name, version, description gibi string alanları için c_ampersand_allocate + memcpy kullan ve _allocated_size alanlarını kaydet.
    // 5. Paket bulunamazsa hata dön (C_AMPERSAND_ERROR_PACKAGE_NOT_FOUND_IN_REPO).
    // 6. Bellek yetersizliği hatalarını (string kopyalama için) raporla.
    // ...

    // Örnek: Şu an sadece bir placeholder.
     return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Paket meta bilgisi alma henüz implemente edilmedi.");

    // Placeholder meta bilgi ataması (manuel bellek tahsisi gerekiyor)
     metadata->name = strdup(package_name); // Değiştirilecek -> c_ampersand_allocate
     metadata->version = strdup("1.0");    // Değiştirilecek -> c_ampersand_allocate
     metadata->description = strdup("Örnek bir paket."); // Değiştirilecek -> c_ampersand_allocate

    // Örnek placeholder bellek tahsisi (TAM DEĞİL, GERÇEK PARSINGDE YAPILMALI)
    size_t name_len = strlen(package_name);
    metadata->name_allocated_size = name_len + 1;
    c_ampersand_result alloc_res = c_ampersand_allocate(metadata->name_allocated_size, (void**)&metadata->name);
    if (alloc_res.code != C_AMPERSAND_OK) { c_ampersand_package_free_metadata(metadata); return alloc_res; }
    memcpy(metadata->name, package_name, metadata->name_allocated_size);

    size_t version_len = strlen("1.0");
    metadata->version_allocated_size = version_len + 1;
    alloc_res = c_ampersand_allocate(metadata->version_allocated_size, (void**)&metadata->version);
    if (alloc_res.code != C_AMPERSAND_OK) { c_ampersand_package_free_metadata(metadata); return alloc_res; }
    memcpy(metadata->version, "1.0", metadata->version_allocated_size);

    size_t desc_len = strlen("Örnek bir paket.");
    metadata->description_allocated_size = desc_len + 1;
    alloc_res = c_ampersand_allocate(metadata->description_allocated_size, (void**)&metadata->description);
    if (alloc_res.code != C_AMPERSAND_OK) { c_ampersand_package_free_metadata(metadata); return alloc_res; }
    memcpy(metadata->description, "Örnek bir paket.", metadata->description_allocated_size);


    c_ampersand_println("REPO NOT: Paket meta bilgisi alındı (simülasyon): %s", package_name);
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// Bir depodaki tüm paketlerin adlarının listesini yazdırır.
// Depo indeksinin daha önce güncellenmiş olması gerekir ve içsel yapıda saklı olmalıdır.
c_ampersand_result c_ampersand_pm_repository_list_packages(const char *url) {
    // Parametre doğrulama
    if (url == NULL || strlen(url) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("REPO NOT: Depodaki paketler listeleniyor: %s", url);

    // --- Implementasyon Taslağı ---
    // 1. url'e karşılık gelen depoyu içsel depo listesinde bul. Depo bulunamazsa hata dön (C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND).
    // 2. Bulunan deponun içsel indeks verisini kontrol et. İndeks verisi henüz indirilmemiş/parse edilmemişse, hata dön (örn. C_AMPERSAND_ERROR_INDEX_NOT_FETCHED).
    // 3. İçsel indeks verisini kullanarak tüm paketlerin adlarını döngüyle gez.
    // 4. Her paket adını c_ampersand_println ile yazdır.
    // ...

    // Örnek: Şu an sadece bir placeholder.
    c_ampersand_println("REPO NOT: Depodaki paket listeleme implementasyon taslağı burada çalışacak.");
    c_ampersand_println("- ornek_paket_1 [simülasyon]"); // Placeholder çıktı
    c_ampersand_println("- baska_paket [simülasyon]"); // Placeholder çıktı


    c_ampersand_println("REPO NOT: Depodaki paketler listelendi (simülasyon): %s", url);
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

#ifndef C_AMPERSAND_REPOSITORY_H
#define C_AMPERSAND_REPOSITORY_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Paket meta bilgisi yapısı için
#include "c&_package_format.h" // c_ampersand_package_metadata tanımı burada olmalıdır.

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// Paket deposu bilgilerini tutan yapı (heap'te ayrılmış stringler içerir)
typedef struct {
    char *url; // Depo URL'i veya yerel dizin yolu (heap'te ayrılmış)
    size_t url_allocated_size; // url stringi için ayrılan boyut (free için)
    // ... diğer depo bilgileri (tipi, öncelik vb.) eklenebilir

    // Depo indeksi verisini tutacak pointer eklenebilir
    // void *index_data;
    // size_t index_data_size;
} c_ampersand_repository_info;


// Depo yönetimi için özel hata kodları (varsayım: c&_stdlib.h'de tanımlı değilse)
#ifndef C_AMPERSAND_ERROR_LIMIT_REACHED
#define C_AMPERSAND_ERROR_LIMIT_REACHED (C_AMPERSAND_ERROR_CUSTOM_START + 10) // Örnek değerler
#endif
#ifndef C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND
#define C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND (C_AMPERSAND_ERROR_CUSTOM_START + 11)
#endif
#ifndef C_AMPERSAND_ERROR_REPOSITORY_ALREADY_ADDED
#define C_AMPERSAND_ERROR_REPOSITORY_ALREADY_ADDED (C_AMPERSAND_ERROR_CUSTOM_START + 12)
#endif
#ifndef C_AMPERSAND_ERROR_INDEX_FETCH_FAILED
#define C_AMPERSAND_ERROR_INDEX_FETCH_FAILED (C_AMPERSAND_ERROR_CUSTOM_START + 13) // Ağ veya G/Ç hatası
#endif
#ifndef C_AMPERSAND_ERROR_INDEX_PARSE_FAILED
#define C_AMPERSAND_ERROR_INDEX_PARSE_FAILED (C_AMPERSAND_ERROR_CUSTOM_START + 14) // İndeks formatı hatası
#endif
#ifndef C_AMPERSAND_ERROR_PACKAGE_NOT_FOUND_IN_REPO
#define C_AMPERSAND_ERROR_PACKAGE_NOT_FOUND_IN_REPO (C_AMPERSAND_ERROR_CUSTOM_START + 15) // Depo indeksinde paket yok
#endif


// Yeni bir paket deposu ekler. URL stringini kopyalar ve saklar.
// url: Eklenecek depo URL'i.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz argüman, limit, zaten ekli, bellek hatası içerebilir.
c_ampersand_result c_ampersand_pm_repository_add(const char *url);

// Bir paket deposunu kaldırır. URL stringini bellekten siler.
// url: Kaldırılacak depo URL'i.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz argüman, bulunamadı hatası içerebilir.
c_ampersand_result c_ampersand_pm_repository_remove(const char *url);

// Tüm yapılandırılmış paket depolarının indekslerini günceller (indirir ve işler).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Ağ, G/Ç, parsing, bellek hataları içerebilir.
c_ampersand_result c_ampersand_pm_repository_update_all();

// Belirli bir paket deposunun indeksini günceller (indirir ve işler).
// url: Güncellenecek depo URL'i.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bulunamadı, ağ, G/Ç, parsing, bellek hataları içerebilir.
c_ampersand_result c_ampersand_pm_repository_update(const char *url);

// Bir depodan belirli bir paket için meta bilgi alır.
// Depo indeksinin daha önce güncellenmiş olması gerekir.
// url: Paketin aranacağı depo URL'i.
// package_name: Meta bilgisi alınacak paketin adı.
// metadata: Paketin meta bilgisinin kopyalanacağı yapı pointer'ı. String alanları heap'te tahsis edilir ve serbest bırakılmalıdır.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz argüman, depo bulunamadı, paket bulunamadı hatası içerebilir.
c_ampersand_result c_ampersand_pm_repository_get_package_metadata(const char *url, const char *package_name, c_ampersand_package_metadata *metadata);

// Bir depodaki tüm paketlerin adlarının listesini yazdırır (şimdilik stdout'a).
// Depo indeksinin daha önce güncellenmiş olması gerekir.
// url: Paketlerin listeleneceği depo URL'i.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz argüman, depo bulunamadı hatası içerebilir.
c_ampersand_result c_ampersand_pm_repository_list_packages(const char *url);

// Depo yönetimi modülünü başlatır (statik diziyi ve sayacı sıfırlar).
c_ampersand_result c_ampersand_pm_repository_init();

// Depo yönetimi modülünü kapatır (saklanan URL stringlerini serbest bırakır).
c_ampersand_result c_ampersand_pm_repository_shutdown();


// ... diğer depo yönetimi fonksiyonları eklenebilir

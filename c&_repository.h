#ifndef C_AMPERSAND_REPOSITORY_H
#define C_AMPERSAND_REPOSITORY_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Paket deposu bilgilerini tutan yapı
typedef struct {
    char *url; // Depo URL'i veya yerel dizin yolu
    // ... diğer depo bilgileri (tipi, öncelik vb.) eklenebilir
} c_ampersand_repository_info;

// Paket meta bilgisini tutan yapı
typedef struct {
    char *name;
    char *version;
    char *description;
    // ... diğer meta bilgileri (bağımlılıklar, yazarlar vb.) eklenebilir
} c_ampersand_package_metadata;

// Yeni bir paket deposu ekler.
c_ampersand_result c_ampersand_pm_repository_add(const char *url);

// Bir paket deposunu kaldırır.
c_ampersand_result c_ampersand_pm_repository_remove(const char *url);

// Tüm yapılandırılmış paket depolarını günceller.
c_ampersand_result c_ampersand_pm_repository_update_all();

// Belirli bir paket deposunun indeksini günceller.
c_ampersand_result c_ampersand_pm_repository_update(const char *url);

// Bir depodan belirli bir paket için meta bilgi alır.
c_ampersand_result c_ampersand_pm_repository_get_package_metadata(const char *url, const char *package_name, c_ampersand_package_metadata *metadata);

// Bir depodaki tüm paketlerin listesini alır.
c_ampersand_result c_ampersand_pm_repository_list_packages(const char *url);

// ... diğer depo yönetimi fonksiyonları eklenebilir

#endif // C_AMPERSAND_REPOSITORY_H
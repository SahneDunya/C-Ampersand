#ifndef C_AMPERSAND_PACKET_MANAGER_H
#define C_AMPERSAND_PACKET_MANAGER_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// C& G/Ç işlemleri ve Handle tipi için (c_ampersand_file_handle)
#include "c&_io_operations.h" // c_ampersand_file_handle tipi u64 olmalıdır.

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için


// Paket bilgisini tutan yapı (Bu modülde doğrudan kullanılmayabilir, arayüz için)
typedef struct {
    char *name;
    size_t name_allocated_size; // name stringi için ayrılan boyut (eğer heap'te tutulursa)
    char *version;
    size_t version_allocated_size; // version stringi için ayrılan boyut (eğer heap'te tutulursa)
    // ... diğer paket bilgileri (bağımlılıklar vb.) eklenebilir
} c_ampersand_package_info;

// Bir paketi verilen isim ve versiyonla kurar.
// package_name: Kurulacak paketin adı.
// version: Kurulacak paketin versiyonu (NULL olabilir, en son versiyonu kur anlamına gelebilir).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Paket zaten kuruluysa, indirme/kurulum/I/O/bellek hataları içerebilir.
c_ampersand_result c_ampersand_pm_install_package(const char *package_name, const char *version);

// Bir paketi kaldırır.
// package_name: Kaldırılacak paketin adı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Paket kurulu değilse, kaldırma/I/O/bellek hataları içerebilir.
c_ampersand_result c_ampersand_pm_uninstall_package(const char *package_name);

// Kurulu paketleri listeler (şimdilik stdout'a yazdırır).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Dizin okuma, dosya okuma, bellek, I/O hataları içerebilir.
c_ampersand_result c_ampersand_pm_list_installed_packages();

// Belirli bir paketin kurulu olup olmadığını kontrol eder.
// package_name: Durumu kontrol edilecek paketin adı.
// Dönüş: Paket kurulu ise true, değilse false. Bellek veya I/O hatası durumunda da false dönebilir (veya hata raporlayabilir).
bool c_ampersand_pm_is_package_installed(const char *package_name);

// Paket depolarını günceller (eğer varsa).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Ağ veya I/O hataları içerebilir.
c_ampersand_result c_ampersand_pm_update_repositories();

// ... diğer paket yöneticisi fonksiyonları eklenebilir (örneğin, arama, bilgi gösterme vb.)

#endif // C_AMPERSAND_PACKET_MANAGER_H

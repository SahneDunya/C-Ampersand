#ifndef C_AMPERSAND_PACKET_MANAGER_H
#define C_AMPERSAND_PACKET_MANAGER_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Paket bilgisini tutan yapı
typedef struct {
    char *name;
    char *version;
    // ... diğer paket bilgileri (bağımlılıklar vb.) eklenebilir
} c_ampersand_package_info;

// Bir paketi belirtir.
typedef char * c_ampersand_package_name;

// Bir paketin versiyonunu belirtir.
typedef char * c_ampersand_package_version;

// Bir paketi kurar.
c_ampersand_result c_ampersand_pm_install_package(c_ampersand_package_name package_name, c_ampersand_package_version version);

// Bir paketi kaldırır.
c_ampersand_result c_ampersand_pm_uninstall_package(c_ampersand_package_name package_name);

// Kurulu paketleri listeler.
c_ampersand_result c_ampersand_pm_list_installed_packages();

// Belirli bir paketin kurulu olup olmadığını kontrol eder.
bool c_ampersand_pm_is_package_installed(c_ampersand_package_name package_name);

// Paket depolarını günceller (eğer varsa).
c_ampersand_result c_ampersand_pm_update_repositories();

// ... diğer paket yöneticisi fonksiyonları eklenebilir (örneğin, arama, bilgi gösterme vb.)

#endif // C_AMPERSAND_PACKET_MANAGER_H
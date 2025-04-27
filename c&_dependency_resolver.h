#ifndef C_AMPERSAND_DEPENDENCY_RESOLVER_H
#define C_AMPERSAND_DEPENDENCY_RESOLVER_H

// Temel C& tipleri ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Paket yönetimi fonksiyonları ve tipleri için (c_ampersand_package_name, c_ampersand_package_version, c_ampersand_pm_is_package_installed)
#include "c&_packet_manager.h"
// Depo fonksiyonları ve tipleri için (c_ampersand_repository, c_ampersand_package_metadata, c_ampersand_pm_repository_get_package_metadata)
#include "c&_repository.h"
// SAHNE64 C API başlığı (Bellek yönetimi için kullanılacak)
#include "sahne.h"

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// Paket bağımlılığını temsil eden yapı
typedef struct {
    // Paket adı ve versiyon kısıtlaması için pointer ve ayrılan boyutunu saklıyoruz
    struct { char *ptr; size_t size; } name; // name için Sahne64'ten ayrılmış string
    struct { char *ptr; size_t size; } version_constraint; // version_constraint için Sahne64'ten ayrılmış string
    // ... diğer bağımlılık bilgileri (isteğe bağlı, önerilen vb.) eklenebilir
} c_ampersand_dependency;

// Bağımlılık kontrolü, çözümleme ve çakışma fonksiyonları
// c_ampersand_result dönüş tipini kullanırlar (c&_stdlib.h'de tanımlı varsayım)


// Bir paketin bağımlılıklarını kontrol eder.
// SAHNE64 bellek yönetimi kullanır.
c_ampersand_result c_ampersand_dependency_check(c_ampersand_package_name package_name, c_ampersand_package_version version);

// Bir paketin bağımlılıklarını çözmeye çalışır.
// Gerekirse kurulması gereken paketleri belirler.
// (Bu fonksiyonun tam implementasyonu kurulacak paketlerin listesini döndürmek için
// daha karmaşık bir yapıya (örneğin c_ampersand_vector<c_ampersand_dependency>) sahip olabilir.)
// SAHNE64 bellek yönetimi kullanır.
c_ampersand_result c_ampersand_dependency_resolve(c_ampersand_package_name package_name, c_ampersand_package_version version);

// İki paket arasında bir bağımlılık çakışması olup olmadığını kontrol eder.
// SAHNE64 bellek yönetimi kullanır.
c_ampersand_result c_ampersand_dependency_check_conflicts(c_ampersand_package_name package_name1, c_ampersand_package_version version1,
                                                         c_ampersand_package_name package_name2, c_ampersand_package_version version2);

// ... diğer bağımlılık çözme ile ilgili fonksiyonlar eklenebilir

#endif // C_AMPERSAND_DEPENDENCY_RESOLVER_H

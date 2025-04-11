#ifndef C_AMPERSAND_DEPENDENCY_RESOLVER_H
#define C_AMPERSAND_DEPENDENCY_RESOLVER_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include "c&_packet_manager.h" // Paket yönetimi fonksiyonları için
#include "c&_repository.h" // Depo fonksiyonları için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Paket bağımlılığını temsil eden yapı
typedef struct {
    c_ampersand_package_name name;
    c_ampersand_package_version version_constraint; // Örneğin, ">=1.0", "=2.1" gibi
    // ... diğer bağımlılık bilgileri (isteğe bağlı, önerilen vb.) eklenebilir
} c_ampersand_dependency;

// Bir paketin bağımlılıklarını kontrol eder.
c_ampersand_result c_ampersand_dependency_check(c_ampersand_package_name package_name, c_ampersand_package_version version);

// Bir paketin bağımlılıklarını çözmeye çalışır ve kurulması gereken paketlerin listesini döndürür.
// (Bu fonksiyonun dönüş tipi daha karmaşık olabilir, örneğin bir dinamik dizi)
c_ampersand_result c_ampersand_dependency_resolve(c_ampersand_package_name package_name, c_ampersand_package_version version);

// İki paket arasında bir bağımlılık çakışması olup olmadığını kontrol eder.
c_ampersand_result c_ampersand_dependency_check_conflicts(c_ampersand_package_name package_name1, c_ampersand_package_version version1,
                                                          c_ampersand_package_name package_name2, c_ampersand_package_version version2);

// ... diğer bağımlılık çözme ile ilgili fonksiyonlar eklenebilir
// (örneğin, döngüsel bağımlılık kontrolü)

#endif // C_AMPERSAND_DEPENDENCY_RESOLVER_H
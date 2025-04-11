#ifndef C_AMPERSAND_DEPENDENCY_ANALYZER_H
#define C_AMPERSAND_DEPENDENCY_ANALYZER_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include "c&_packet_manager.h" // Paket isimleri için
#include "c&_repository.h" // Paket meta bilgisi almak için
#include <stdbool.h>

// Bağımlılık grafiğindeki bir düğümü temsil eder.
typedef struct c_ampersand_dependency_node {
    c_ampersand_package_name package_name;
    // ... diğer düğüm bilgileri (örneğin, ziyaret durumu) eklenebilir
    struct c_ampersand_dependency_node **dependencies;
    size_t dependency_count;
    size_t dependency_capacity;
} c_ampersand_dependency_node;

// Bağımlılık analiz sonuçlarını tutan yapı (isteğe bağlı)
typedef struct {
    bool has_circular_dependency;
    // ... diğer analiz sonuçları eklenebilir
} c_ampersand_dependency_analysis_result;

// Tüm bilinen paketler için bağımlılık grafiğini oluşturur.
c_ampersand_result c_ampersand_dependency_analyzer_build_graph();

// Belirli bir paket için bağımlılık grafiğini oluşturur (gerekirse).
c_ampersand_result c_ampersand_dependency_analyzer_build_graph_for_package(c_ampersand_package_name package_name);

// Bağımlılık grafiğinde döngüsel bağımlılık olup olmadığını kontrol eder.
c_ampersand_result c_ampersand_dependency_analyzer_detect_cycles(c_ampersand_dependency_analysis_result *result);

// Bir paket için tüm bağımlılıkları (transitive dahil) alır.
c_ampersand_result c_ampersand_dependency_analyzer_get_all_dependencies(c_ampersand_package_name package_name);

// Bağımlılık grafiğini serbest bırakır.
c_ampersand_result c_ampersand_dependency_analyzer_free_graph();

// ... diğer bağımlılık analizi ile ilgili fonksiyonlar eklenebilir
// (örneğin, belirli bir paketin bağımlılık ağacını gösterme)

#endif // C_AMPERSAND_DEPENDENCY_ANALYZER_H
#ifndef C_AMPERSAND_DEPENDENCY_ANALYZER_H
#define C_AMPERSAND_DEPENDENCY_ANALYZER_H

// Temel C& tipleri ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Paket isimleri için (c_ampersand_package_name)
#include "c&_packet_manager.h"
// Paket meta bilgisi almak için (bu dosyada doğrudan kullanılmıyor ama konsept gereği dahil)
#include "c&_repository.h"
// SAHNE64 C API başlığı (Bellek yönetimi için kullanılacak)
#include "sahne.h"

#include <stdbool.h> // bool için
#include <stddef.h>  // size_t için

// Bağımlılık grafiğindeki bir düğümü temsil eder.
typedef struct c_ampersand_dependency_node {
    c_ampersand_package_name package_name; // Paketin adı (heap'te ayrılmış string)
    size_t name_allocated_size; // package_name için ayrılan boyut (release için)

    struct c_ampersand_dependency_node **dependencies; // Bu paketin bağımlı olduğu düğümler dizisi (heap'te ayrılmış dizi)
    size_t dependency_count;    // Şu anki bağımlılık sayısı
    size_t dependency_capacity; // Bağımlılıklar dizisinin şu anki kapasitesi
    size_t dependencies_allocated_size; // dependencies dizisi için ayrılan boyut (release için)

    // ... diğer düğüm bilgileri (örneğin, DFS için ziyaret durumu gibi, ancak DFS fonksiyonu dışarıdan state yönetiyor)
} c_ampersand_dependency_node;

// Bağımlılık analiz sonuçlarını tutan yapı (isteğe bağlı)
typedef struct {
    bool has_circular_dependency;
    // ... diğer analiz sonuçları eklenebilir
} c_ampersand_dependency_analysis_result;

// Tüm bilinen paketler için bağımlılık grafiğini oluşturur.
// SAHNE64 bellek tahsisini kullanır.
c_ampersand_result c_ampersand_dependency_analyzer_build_graph();

// Belirli bir paket için bağımlılık grafiğini oluşturur (gerekirse).
// SAHNE64 bellek tahsisini kullanır.
c_ampersand_result c_ampersand_dependency_analyzer_build_graph_for_package(c_ampersand_package_name package_name);

// Bağımlılık grafiğinde döngüsel bağımlılık olup olmadığını kontrol eder (DFS kullanır).
c_ampersand_result c_ampersand_dependency_analyzer_detect_cycles(c_ampersand_dependency_analysis_result *result);

// Bir paket için tüm bağımlılıkları (transitive dahil) alır.
// NOT: Mevcut implementasyon sadece doğrudan bağımlılıkları listeler.
c_ampersand_result c_ampersand_dependency_analyzer_get_all_dependencies(c_ampersand_package_name package_name);

// Bağımlılık grafiğini ve kullandığı belleği serbest bırakır.
// SAHNE64 bellek serbest bırakmayı kullanır.
c_ampersand_result c_ampersand_dependency_analyzer_free_graph();

// ... diğer bağımlılık analizi ile ilgili fonksiyonlar eklenebilir
// (örneğin, belirli bir paketin bağımlılık ağacını gösterme)

#endif // C_AMPERSAND_DEPENDENCY_ANALYZER_H

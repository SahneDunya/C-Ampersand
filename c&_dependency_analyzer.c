#include "c&_dependency_analyzer.h"
#include "c&_packet_manager.h" // Varsayım: c_ampersand_package_name burada tanımlı
#include "c&_repository.h"     // Kullanılmıyor ama konsept gereği dahil ediliyor olabilir
#include "c&_stdlib.h"         // Varsayım: c_ampersand_result, C_AMPERSAND_* hataları, c_ampersand_println burada tanımlı
#include "sahne.h"             // SAHNE64 C API başlığı (Bellek yönetimi için)


#include <string.h>            // strcmp, strlen için
#include <stdbool.h>           // bool için
#include <stddef.h>            // size_t, NULL için

// --- Dahili Veri Yapıları ve Fonksiyonlar ---

// Bağımlılık grafiği (basit statik dizi limitli bir implementasyon)
#define MAX_PACKAGES 100
static c_ampersand_dependency_node *package_nodes[MAX_PACKAGES];
static size_t package_count = 0;

// Bir paket adı için düğümü bulur.
static c_ampersand_dependency_node *find_node(c_ampersand_package_name package_name) {
    for (size_t i = 0; i < package_count; ++i) {
        // package_nodes[i] NULL değil ve package_name'i doğru ayrılmış ve NULL değilse karşılaştır
        if (package_nodes[i] != NULL && package_nodes[i]->package_name != NULL &&
            strcmp(package_nodes[i]->package_name, package_name) == 0) {
            return package_nodes[i];
        }
    }
    return NULL; // Bulunamadı
}

// Yeni bir bağımlılık düğümü oluşturur.
// SAHNE64 bellek tahsisini kullanır.
static c_ampersand_dependency_node *create_node(c_ampersand_package_name package_name) {
    c_ampersand_dependency_node *node = NULL;
    // Düğüm struct'ı için bellek tahsis et
    sahne_error_t sahne_err = sahne_mem_allocate(sizeof(c_ampersand_dependency_node), (void**)&node);
    if (sahne_err != SAHNE_SUCCESS) {
        // Bellek hatası
        return NULL;
    }

    // Düğüm struct'ını sıfırla (güvenlik için)
    memset(node, 0, sizeof(c_ampersand_dependency_node));


    // Paket adı stringi için bellek tahsis et
    size_t name_len = strlen(package_name);
    size_t name_allocation_size = name_len + 1; // Null sonlandırıcı dahil
    char *name_ptr = NULL;
    sahne_err = sahne_mem_allocate(name_allocation_size, (void**)&name_ptr);

    if (sahne_err != SAHNE_SUCCESS) {
        // String bellek hatası, daha önce tahsis edilen düğümü serbest bırak
        sahne_mem_release(node, sizeof(c_ampersand_dependency_node));
        return NULL;
    }

    // Stringi kopyala
    memcpy(name_ptr, package_name, name_allocation_size);

    // Düğüm alanlarını ayarla
    node->package_name = name_ptr;
    node->name_allocated_size = name_allocation_size; // Ayrılan boyutu kaydet

    node->dependencies = NULL;
    node->dependency_count = 0;
    node->dependency_capacity = 0;
    node->dependencies_allocated_size = 0; // Başlangıçta bağımlılık dizisi için ayrılan boyut 0

    return node;
}

// Bir düğüme bağımlılık ekler.
// SAHNE64 bellek tahsisini (realloc yerine) kullanır.
static c_ampersand_result add_dependency(c_ampersand_dependency_node *node, c_ampersand_package_name dependency_name) {
    if (node == NULL || dependency_name == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    // Bağımlılıklar dizisi için kapasite yeterli değilse
    if (node->dependency_count >= node->dependency_capacity) {
        size_t old_capacity = node->dependency_capacity;
        size_t new_capacity = (old_capacity == 0) ? 4 : old_capacity * 2; // Yeni kapasiteyi hesapla
        size_t new_allocation_size = sizeof(c_ampersand_dependency_node *) * new_capacity; // Yeni boyutu hesapla

        c_ampersand_dependency_node **new_dependencies = NULL;
        // Yeni belleği tahsis et
        sahne_error_t sahne_err = sahne_mem_allocate(new_allocation_size, (void**)&new_dependencies);

        if (sahne_err != SAHNE_SUCCESS) {
            // Bellek hatası
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
        }

        // Eski veriyi yeni konuma kopyala (sadece mevcut elemanlar kadar)
        if (node->dependencies != NULL) {
             memcpy(new_dependencies, node->dependencies, sizeof(c_ampersand_dependency_node *) * node->dependency_count);
        }

        // Eski belleği serbest bırak (Eğer daha önce bellek tahsis edildiyse)
        if (node->dependencies != NULL) {
            // sahne_error_t release_err =
            sahne_mem_release(node->dependencies, node->dependencies_allocated_size);
            // Serbest bırakma hataları genellikle kritik değildir ama loglanabilir.
        }

        // Düğümdeki pointer ve boyut bilgilerini güncelle
        node->dependencies = new_dependencies;
        node->dependency_capacity = new_capacity;
        node->dependencies_allocated_size = new_allocation_size; // Yeni ayrılan boyutu kaydet
    }

    // Bağımlılık düğümünü bul veya oluştur
    c_ampersand_dependency_node *dependency_node = find_node(dependency_name);
    if (dependency_node == NULL) {
        dependency_node = create_node(dependency_name); // Yeni düğümü oluştur (bu da Sahne64 bellek kullanır)
        if (dependency_node == NULL) {
            // Bağımlılık düğümü oluşturulamadıysa bellek hatası
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
        }
        // Yeni oluşturulan düğümü genel listeye ekle
        if (package_count >= MAX_PACKAGES) {
            // Statik dizi limitine ulaşıldı
            c_ampersand_println("Uyarı: MAX_PACKAGES limitine ulaşıldı (%zu). '%s' paketi eklenemedi.", MAX_PACKAGES, dependency_name);
            // Oluşturulan düğüm belleğini serbest bırak (genel listeye eklenemeyeceği için)
            // create_node içinde string ve node belleği ayrılmıştı, free_graph gibi serbest bırakalım.
            if (dependency_node->package_name) sahne_mem_release(dependency_node->package_name, dependency_node->name_allocated_size);
            // Bağımlılık dizisi boşsa NULL olur, release NULL pointer için güvenli varsayalım
            if (dependency_node->dependencies) sahne_mem_release(dependency_node->dependencies, dependency_node->dependencies_allocated_size);
            sahne_mem_release(dependency_node, sizeof(c_ampersand_dependency_node));

            return (c_ampersand_result){C_AMPERSAND_ERROR_LIMIT_EXCEEDED}; // Yeni bir hata kodu önerisi
        }
        package_nodes[package_count++] = dependency_node; // Genel listeye ekle
    }

    // Bağımlılık düğüm pointer'ını bağımlılıklar dizisine ekle
    node->dependencies[node->dependency_count++] = dependency_node;

    return C_AMPERSAND_OK_RESULT;
}

// --- Bağımlılık Analizi Fonksiyonlarının Implementasyonu ---

c_ampersand_result c_ampersand_dependency_analyzer_build_graph() {
    c_ampersand_println("Bağımlılık grafiği oluşturuluyor...");

    // Önceki grafiği temizle (varsa)
     c_ampersand_dependency_analyzer_free_graph(); // Eğer build_graph birden fazla çağrılabilirse eklenmeli

    // Burada gerçek paket yöneticisinden kurulu paketlerin listesi alınmalı
    // veya repository modülünden tüm bilinen paketler ve bağımlılıkları sorgulanmalı.
    // Örnek olarak statik paketler ve bağımlılıklar varsayılıyor.

    // Örnek paketler (Bunların repository'den geldiğini varsayalım)
    const char *packages[] = {"paket_a", "paket_b", "paket_c"};
    const size_t num_packages = sizeof(packages) / sizeof(packages[0]);

    // Önce tüm düğümleri oluştur (bağımlılıkları henüz eklemeden)
    for (size_t i = 0; i < num_packages; ++i) {
        // MAX_PACKAGES limitini kontrol et
        if (package_count >= MAX_PACKAGES) {
             c_ampersand_println("Uyarı: MAX_PACKAGES limitine ulaşıldı (%zu). Tüm paketler grafiğe eklenemedi.", MAX_PACKAGES);
             break; // Limite ulaşıldı, döngüyü kır
        }
        c_ampersand_dependency_node *node = create_node(packages[i]);
        if (node == NULL) {
            // Bellek hatası
             // Kısmi oluşturulmuş grafiği temizleme logic'i eklenebilir burada
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        package_nodes[package_count++] = node; // Genel listeye ekle
    }

    // Şimdi bağımlılıkları ekle (düğümlerin hepsi artık genel listede)
     for (size_t i = 0; i < package_count; ++i) {
         c_ampersand_dependency_node *node = package_nodes[i];
         // Bu noktada package_nodes[i] null olmamalıdır, çünkü create_node başarısız olsaydı dönmüştük.

         // Her paket için bağımlılıkları repository'den alıp grafiğe ekle
        // Örnek bağımlılıklar (repository'den geliyormuş gibi):
        if (strcmp(node->package_name, "paket_a") == 0) {
            c_ampersand_result add_res = add_dependency(node, "paket_b");
             if (add_res.code != C_AMPERSAND_OK) return add_res; // Hata durumunda dön
        } else if (strcmp(node->package_name, "paket_b") == 0) {
            c_ampersand_result add_res = add_dependency(node, "paket_c");
             if (add_res.code != C_AMPERSAND_OK) return add_res; // Hata durumunda dön
        } else if (strcmp(node->package_name, "paket_c") == 0) {
            c_ampersand_result add_res = add_dependency(node, "paket_a"); // Döngüsel bağımlılık örneği
             if (add_res.code != C_AMPERSAND_OK) return add_res; // Hata durumunda dön
        }
        // repository modülü kullanılarak paketlerin gerçek bağımlılıkları sorgulanabilir
         c_ampersand_vector dependencies; // Varsayım: c&_repository'den bağımlılık listesi alır
         c_ampersand_result repo_res = c_ampersand_repository_get_dependencies(node->package_name, &dependencies);
         if (repo_res.code == C_AMPERSAND_OK) {
            for (size_t j = 0; j < c_ampersand_vector_size(&dependencies); ++j) {
               c_ampersand_package_name dep_name; // Varsayım: Vektörden package name stringi alınır
               c_ampersand_vector_get(&dependencies, j, &dep_name);
               c_ampersand_result add_res = add_dependency(node, dep_name);
               if (add_res.code != C_AMPERSAND_OK) { c_ampersand_vector_destroy(&dependencies); return add_res; }
            }
            c_ampersand_vector_destroy(&dependencies);
         } else {
        //    // Bağımlılık alma hatası
        //    // Grafiği temizle ve hata dön
            c_ampersand_dependency_analyzer_free_graph();
            return repo_res;
         }
     }


    c_ampersand_println("Bağımlılık grafiği oluşturuldu.");
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_analyzer_build_graph_for_package(c_ampersand_package_name package_name) {
    // Daha gelişmiş bir implementasyon, sadece istenen paket ve onun bağımlılıkları
    // için grafiği kısmen oluşturabilir. Bu örnekte basitçe tam grafiği oluşturur.
    return c_ampersand_dependency_analyzer_build_graph();
}

// Yardımcı fonksiyon: Döngüsel bağımlılık tespiti için DFS
// Not: Bu DFS implementasyonu, package_nodes dizisinin contiguous (bitişik)
// bellekte olduğunu varsayar (statik dizi olduğu için geçerli).
static bool has_cycle_recursive(c_ampersand_dependency_node *node, bool *visited, bool *recursion_stack) {
    // Düğümün package_nodes dizisindeki indeksini hesapla
    // NULL pointer kontrolü ekleyelim (olmamalı ama güvenlik için)
    if (node == NULL || package_count == 0) return false;
    size_t node_index = node - package_nodes[0];
    if (node_index >= package_count) return false; // Geçersiz indeks (should not happen if logic is correct)


    visited[node_index] = true;
    recursion_stack[node_index] = true;

    for (size_t i = 0; i < node->dependency_count; ++i) {
        c_ampersand_dependency_node *neighbor = node->dependencies[i];
         // Komşu düğümün de NULL olabileceği durumları ele al (add_dependency hatası gibi)
         if (neighbor == NULL) continue;

         size_t neighbor_index = neighbor - package_nodes[0];
         if (neighbor_index >= package_count) continue; // Geçersiz komşu indeksi (should not happen)

        if (!visited[neighbor_index]) {
            if (has_cycle_recursive(neighbor, visited, recursion_stack)) {
                return true; // Özyinelemeli çağrıda döngü bulundu
            }
        } else if (recursion_stack[neighbor_index]) {
            return true; // Döngü bulundu (komşu şu anki özyineleme yığınında)
        }
    }

    recursion_stack[node_index] = false; // Bu düğümü özyineleme yığınından çıkar
    return false; // Bu düğümden başlayan yolda döngü bulunamadı
}

c_ampersand_result c_ampersand_dependency_analyzer_detect_cycles(c_ampersand_dependency_analysis_result *result) {
    // Genel grafiğin zaten oluşturulduğunu varsayıyoruz
    if (package_count == 0) {
        if (result != NULL) {
            result->has_circular_dependency = false;
        }
        return C_AMPERSAND_OK_RESULT; // Boş grafikte döngü yoktur
    }

    // Ziyaret durumunu ve özyineleme yığınını saklamak için diziler
    // package_count <= MAX_PACKAGES olduğu için statik diziler kullanılabilir
    bool visited[MAX_PACKAGES] = {false};
    bool recursion_stack[MAX_PACKAGES] = {false};

    // Grafikteki her düğüm için DFS başlat
    for (size_t i = 0; i < package_count; ++i) {
        if (package_nodes[i] != NULL && !visited[i]) { // Düğüm null değilse ve daha önce ziyaret edilmemişse
            if (has_cycle_recursive(package_nodes[i], visited, recursion_stack)) {
                // Döngü bulundu
                if (result != NULL) {
                    result->has_circular_dependency = true;
                }
                c_ampersand_println("Hata: Döngüsel bağımlılık tespit edildi.");
                // Varsayım: C_AMPERSAND_ERROR_DEPENDENCY_CYCLE c&_stdlib.h'de tanımlı
                return (c_ampersand_result){C_AMPERSAND_ERROR_DEPENDENCY_CYCLE};
            }
        }
    }

    // Döngü bulunamadı
    if (result != NULL) {
        result->has_circular_dependency = false;
    }
    c_ampersand_println("Döngüsel bağımlılık bulunamadı.");
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_analyzer_get_all_dependencies(c_ampersand_package_name package_name) {
    // Genel grafiğin zaten oluşturulduğunu varsayıyoruz
    c_ampersand_dependency_node *start_node = find_node(package_name);
    if (start_node == NULL) {
        c_ampersand_println("Paket bulunamadı: %s", package_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_println("'%s' paketinin tüm bağımlılıkları:", package_name);
    // Burada BFS veya DFS ile tüm BAĞIMLILIKLARI (transitive dahil) bulup listeleyebilirsiniz.
    // Bu örnekte SADECE DOĞRUDAN bağımlılıklar gösteriliyor ve bir Result dönülmüyor,
    // sadece çıktı veriliyor. Tam liste döndürmek için başka bir mekanizma (örn. vektör) gerekir.
    for (size_t i = 0; i < start_node->dependency_count; ++i) {
        // Bağımlılık düğümünün NULL olabileceği durumu kontrol et
        if (start_node->dependencies[i] != NULL && start_node->dependencies[i]->package_name != NULL) {
            c_ampersand_println("- %s", start_node->dependencies[i]->package_name);
        }
    }
     // Transitive bağımlılıkları bulmak için buraya grafik geçiş algoritması eklenebilir.

    return C_AMPERSAND_OK_RESULT; // Fonksiyon başarıyla tamamlandıysa başarı döner
}

c_ampersand_result c_ampersand_dependency_analyzer_free_graph() {
    // Genel grafiğin oluşturulduğunu varsayıyoruz
    for (size_t i = 0; i < package_count; ++i) {
        // Düğümün null olup olmadığını kontrol et (hatalı oluşturulmuş olabilir)
        if (package_nodes[i] != NULL) {
            // Paket adı stringini serbest bırak
            if (package_nodes[i]->package_name != NULL) {
                  sahne_error_t err =
                 sahne_mem_release(package_nodes[i]->package_name, package_nodes[i]->name_allocated_size);
                 // Log hata?
                 package_nodes[i]->package_name = NULL; // İşaretçiyi NULL yap
                 package_nodes[i]->name_allocated_size = 0;
            }

            // Bağımlılıklar dizisini serbest bırak
            if (package_nodes[i]->dependencies != NULL) {
                  sahne_error_t err =
                 sahne_mem_release(package_nodes[i]->dependencies, package_nodes[i]->dependencies_allocated_size);
                 // Log hata?
                 package_nodes[i]->dependencies = NULL; // İşaretçiyi NULL yap
                 package_nodes[i]->dependency_count = 0;
                 package_nodes[i]->dependency_capacity = 0;
                 package_nodes[i]->dependencies_allocated_size = 0;
            }

            // Düğüm struct'ını serbest bırak
            sahne_error_t err =
            sahne_mem_release(package_nodes[i], sizeof(c_ampersand_dependency_node));
            // Log hata?
            package_nodes[i] = NULL; // Genel dizideki pointer'ı NULL yap
        }
    }
    package_count = 0; // Paket sayacını sıfırla
    c_ampersand_println("Bağımlılık grafiği serbest bırakıldı.");
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// ... diğer bağımlılık analizi ile ilgili fonksiyonlar eklenebilir

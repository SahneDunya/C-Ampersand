#include "c&_dependency_analyzer.h"
#include "c&_packet_manager.h"
#include "c&_repository.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- Dahili Veri Yapıları ve Fonksiyonlar ---

// Bağımlılık grafiği (basit bir hash map veya dizi olarak implemente edilebilir)
#define MAX_PACKAGES 100
static c_ampersand_dependency_node *package_nodes[MAX_PACKAGES];
static size_t package_count = 0;

// Bir paket adı için düğümü bulur.
static c_ampersand_dependency_node *find_node(c_ampersand_package_name package_name) {
    for (size_t i = 0; i < package_count; ++i) {
        if (strcmp(package_nodes[i]->package_name, package_name) == 0) {
            return package_nodes[i];
        }
    }
    return NULL;
}

// Yeni bir bağımlılık düğümü oluşturur.
static c_ampersand_dependency_node *create_node(c_ampersand_package_name package_name) {
    c_ampersand_dependency_node *node = malloc(sizeof(c_ampersand_dependency_node));
    if (node == NULL) {
        return NULL;
    }
    node->package_name = strdup(package_name);
    node->dependencies = NULL;
    node->dependency_count = 0;
    node->dependency_capacity = 0;
    return node;
}

// Bir düğüme bağımlılık ekler.
static c_ampersand_result add_dependency(c_ampersand_dependency_node *node, c_ampersand_package_name dependency_name) {
    if (node->dependency_count >= node->dependency_capacity) {
        node->dependency_capacity = (node->dependency_capacity == 0) ? 4 : node->dependency_capacity * 2;
        c_ampersand_dependency_node **new_dependencies = realloc(node->dependencies, sizeof(c_ampersand_dependency_node *) * node->dependency_capacity);
        if (new_dependencies == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        node->dependencies = new_dependencies;
    }
    c_ampersand_dependency_node *dependency_node = find_node(dependency_name);
    if (dependency_node == NULL) {
        dependency_node = create_node(dependency_name);
        if (dependency_node == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        if (package_count >= MAX_PACKAGES) {
            c_ampersand_println("Uyarı: Çok fazla paket.");
            return C_AMPERSAND_OK_RESULT; // Sınıra ulaşıldı
        }
        package_nodes[package_count++] = dependency_node;
    }
    node->dependencies[node->dependency_count++] = dependency_node;
    return C_AMPERSAND_OK_RESULT;
}

// --- Bağımlılık Analizi Fonksiyonlarının Implementasyonu ---

c_ampersand_result c_ampersand_dependency_analyzer_build_graph() {
    c_ampersand_println("Bağımlılık grafiği oluşturuluyor...");

    // Burada kurulu paketlerin listesi alınmalı veya tüm bilinen paketler için işlem yapılmalı.
    // Örnek olarak statik paketler varsayılıyor.
    const char *packages[] = {"paket_a", "paket_b", "paket_c"};
    const size_t num_packages = sizeof(packages) / sizeof(packages[0]);

    for (size_t i = 0; i < num_packages; ++i) {
        c_ampersand_dependency_node *node = create_node(packages[i]);
        if (node == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        if (package_count >= MAX_PACKAGES) {
            c_ampersand_println("Uyarı: Çok fazla paket.");
            free(node->package_name);
            free(node);
            break;
        }
        package_nodes[package_count++] = node;

        // Burada her paket için bağımlılıklar alınmalı ve grafiğe eklenmeli.
        // Örnek bağımlılıklar:
        if (strcmp(packages[i], "paket_a") == 0) {
            add_dependency(node, "paket_b");
        } else if (strcmp(packages[i], "paket_b") == 0) {
            add_dependency(node, "paket_c");
        } else if (strcmp(packages[i], "paket_c") == 0) {
            add_dependency(node, "paket_a"); // Döngüsel bağımlılık örneği
        }
    }

    c_ampersand_println("Bağımlılık grafiği oluşturuldu.");
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_analyzer_build_graph_for_package(c_ampersand_package_name package_name) {
    // İstenen paket ve bağımlılıkları için grafiği oluşturur (gerekirse).
    // Bu örnekte temel build_graph fonksiyonu kullanılıyor.
    return c_ampersand_dependency_analyzer_build_graph();
}

// Yardımcı fonksiyon: Döngüsel bağımlılık tespiti için DFS
static bool has_cycle_recursive(c_ampersand_dependency_node *node, bool *visited, bool *recursion_stack) {
    visited[node - package_nodes[0]] = true;
    recursion_stack[node - package_nodes[0]] = true;

    for (size_t i = 0; i < node->dependency_count; ++i) {
        c_ampersand_dependency_node *neighbor = node->dependencies[i];
        if (!visited[neighbor - package_nodes[0]]) {
            if (has_cycle_recursive(neighbor, visited, recursion_stack)) {
                return true;
            }
        } else if (recursion_stack[neighbor - package_nodes[0]]) {
            return true; // Döngü bulundu
        }
    }

    recursion_stack[node - package_nodes[0]] = false;
    return false;
}

c_ampersand_result c_ampersand_dependency_analyzer_detect_cycles(c_ampersand_dependency_analysis_result *result) {
    if (package_count == 0) {
        if (result != NULL) {
            result->has_circular_dependency = false;
        }
        return C_AMPERSAND_OK_RESULT;
    }

    bool visited[MAX_PACKAGES] = {false};
    bool recursion_stack[MAX_PACKAGES] = {false};

    for (size_t i = 0; i < package_count; ++i) {
        if (!visited[i]) {
            if (has_cycle_recursive(package_nodes[i], visited, recursion_stack)) {
                if (result != NULL) {
                    result->has_circular_dependency = true;
                }
                c_ampersand_println("Hata: Döngüsel bağımlılık tespit edildi.");
                return (c_ampersand_result){C_AMPERSAND_ERROR_DEPENDENCY_CYCLE};
            }
        }
    }

    if (result != NULL) {
        result->has_circular_dependency = false;
    }
    c_ampersand_println("Döngüsel bağımlılık bulunamadı.");
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_analyzer_get_all_dependencies(c_ampersand_package_name package_name) {
    c_ampersand_dependency_node *start_node = find_node(package_name);
    if (start_node == NULL) {
        c_ampersand_println("Paket bulunamadı: %s", package_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_println("'%s' paketinin tüm bağımlılıkları:", package_name);
    // Burada BFS veya DFS ile tüm bağımlılıkları bulup listeleyebilirsiniz.
    // Bu örnekte sadece doğrudan bağımlılıklar gösteriliyor.
    for (size_t i = 0; i < start_node->dependency_count; ++i) {
        c_ampersand_println("- %s", start_node->dependencies[i]->package_name);
    }

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_analyzer_free_graph() {
    for (size_t i = 0; i < package_count; ++i) {
        free(package_nodes[i]->package_name);
        free(package_nodes[i]->dependencies);
        free(package_nodes[i]);
    }
    package_count = 0;
    c_ampersand_println("Bağımlılık grafiği serbest bırakıldı.");
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer bağımlılık analizi ile ilgili fonksiyonların implementasyonları buraya gelebilir
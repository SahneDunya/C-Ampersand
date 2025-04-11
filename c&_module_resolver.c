#include "c&_module_resolver.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası (dinamik bağlama için)
#include "c&_io.h"     // Dosya sistemi işlemleri için
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- Dahili Sabitler ve Veri Yapıları ---

#define MAX_SEARCH_PATHS 16
static char *search_paths[MAX_SEARCH_PATHS];
static int num_search_paths = 0;

// --- Yardımcı Fonksiyonlar ---

// Bir modül adından tam dosya yolunu oluşturur.
static char *construct_module_path(const char *module_name) {
    // Örnek modül dosya adı uzantısı
    const char *module_extension = ".s64mod"; // Sahne64 modül uzantısı varsayımı

    for (int i = 0; i < num_search_paths; ++i) {
        size_t path_len = strlen(search_paths[i]);
        size_t name_len = strlen(module_name);
        size_t ext_len = strlen(module_extension);
        size_t total_len = path_len + 1 + name_len + ext_len + 1; // yol/modul.uzantı\0

        char *full_path = malloc(total_len);
        if (full_path == NULL) {
            return NULL;
        }
        snprintf(full_path, total_len, "%s/%s%s", search_paths[i], module_name, module_extension);

        // Dosyanın var olup olmadığını kontrol et (Sahne64 API'sını kullanarak)
        sahne64_fd_t fd = sahne64_open(full_path, SAHNE64_O_RDONLY, 0);
        if (fd >= 0) {
            sahne64_close(fd);
            return full_path;
        }
        free(full_path);
    }
    return NULL;
}

// --- Modül Çözme Fonksiyonlarının Implementasyonu ---

c_ampersand_result c_ampersand_module_load(c_ampersand_module_name module_name, c_ampersand_module_handle *handle) {
    if (module_name == NULL || handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    char *module_path = construct_module_path(module_name);
    if (module_path == NULL) {
        c_ampersand_println("Hata: Modül bulunamadı: %s", module_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_MODULE_NOT_FOUND};
    }

    // Sahne64 API'sını kullanarak modülü yükle
    c_ampersand_module_handle loaded_module = sahne64_module_load(module_path);
    free(module_path);

    if (loaded_module == NULL) {
        c_ampersand_println("Hata: Modül yüklenirken bir sorun oluştu: %s", module_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_MODULE_LOAD_FAILED};
    }

    *handle = loaded_module;
    c_ampersand_println("Modül yüklendi: %s (%p)", module_name, loaded_module);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_module_unload(c_ampersand_module_handle handle) {
    if (handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    if (sahne64_module_unload(handle) < 0) {
        c_ampersand_println("Hata: Modül kaldırılırken bir sorun oluştu: %p", handle);
        return (c_ampersand_result){C_AMPERSAND_ERROR_MODULE_UNLOAD_FAILED};
    }

    c_ampersand_println("Modül kaldırıldı: %p", handle);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_module_get_symbol(c_ampersand_module_handle handle, const char *symbol_name, void **symbol_address) {
    if (handle == NULL || symbol_name == NULL || symbol_address == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    void *symbol = sahne64_module_get_symbol(handle, symbol_name);
    if (symbol == NULL) {
        c_ampersand_println("Hata: Sembol bulunamadı: %s", symbol_name);
        return (c_ampersand_result){C_AMPERSAND_ERROR_SYMBOL_NOT_FOUND};
    }

    *symbol_address = symbol;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_module_add_search_path(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    if (num_search_paths >= MAX_SEARCH_PATHS) {
        c_ampersand_println("Uyarı: Çok fazla arama yolu eklenemez.");
        return C_AMPERSAND_OK_RESULT; // Limit aşıldı
    }

    search_paths[num_search_paths] = strdup(path);
    if (search_paths[num_search_paths] == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    num_search_paths++;

    c_ampersand_println("Arama yolu eklendi: %s", path);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_module_init() {
    // Başlangıçta bazı varsayılan arama yolları eklenebilir.
    // Örneğin, sistem modül dizinleri.
    c_ampersand_module_add_search_path("./modules"); // Mevcut dizindeki "modules" klasörü
    c_ampersand_println("Modül çözücü başlatıldı.");
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_module_shutdown() {
    // Arama yollarını serbest bırak
    for (int i = 0; i < num_search_paths; ++i) {
        free(search_paths[i]);
    }
    num_search_paths = 0;
    c_ampersand_println("Modül çözücü kapatıldı.");
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer modül çözme ile ilgili fonksiyonların implementasyonları buraya gelebilir
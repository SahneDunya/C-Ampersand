#include "c&_memory.h"
// c_ampersand_result yapısı, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Bellek yönetimi fonksiyonları ve hata tipleri için)
#include "sahne.h"
// Bellek hook tip tanımları ve extern hook işaretçileri için
#include "c&_memory_hooks.h"

#include <stddef.h> // size_t, NULL için

// Helper function to map Sahne64 error codes to C& error codes
// ... (map_sahne_error_to_camper_error fonksiyonu buraya eklendi veya başka bir common yerde varsayılır)

// Belirli boyutta bellek ayırır.
// Önce hook kontrol edilir, ayarlanmışsa hook çağrılır. Yoksa varsayılan Sahne64 API'si kullanılır.
c_ampersand_result c_ampersand_allocate(size_t size, void **ptr) {
    // Önceki NULL ptr kontrolü burada da gerekli
    if (ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Eğer bir bellek ayırma hook'u ayarlanmışsa, onu çağır
    if (c_ampersand_allocate_hook != NULL) {
        // Hook fonksiyonu zaten c_ampersand_result döner ve ptr'yi doldurur.
        return c_ampersand_allocate_hook(size, ptr);
    }

    // Hook ayarlanmamışsa, varsayılan Sahne64 bellek tahsisini kullan
    void *allocated_memory = NULL;
    sahne_error_t sahne_err = sahne_mem_allocate(size, &allocated_memory);

    if (sahne_err != SAHNE_SUCCESS) {
        *ptr = NULL; // Hata durumunda ptr'yi NULL yap
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    *ptr = allocated_memory;
    return C_AMPERSAND_OK_RESULT;
}

// Daha önce c_ampersand_allocate (veya hook'u) ile ayrılmış olan belleği serbest bırakır.
// Önce hook kontrol edilir, ayarlanmışsa hook çağrılır. Yoksa varsayılan Sahne64 API'si kullanılır.
c_ampersand_result c_ampersand_free(void *ptr, size_t size) {
    // Önceki NULL ptr ve size 0 kontrolleri burada da gerekli
    if (ptr == NULL) {
        return C_AMPERSAND_OK_RESULT;
    }
    if (size == 0) { // ptr NULL değilken size 0
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Eğer bir bellek serbest bırakma hook'u ayarlanmışsa, onu çağır
    if (c_ampersand_free_hook != NULL) {
        // Hook fonksiyonu zaten c_ampersand_result döner.
        return c_ampersand_free_hook(ptr, size);
    }

    // Hook ayarlanmamışsa, varsayılan Sahne64 bellek serbest bırakmayı kullan
    sahne_error_t sahne_err = sahne_mem_release(ptr, size);

    if (sahne_err != SAHNE_SUCCESS) {
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    return C_AMPERSAND_OK_RESULT;
}

// ... diğer bellek yönetimi fonksiyonları buraya eklenebilir

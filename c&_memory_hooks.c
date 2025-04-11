#include "c&_memory_hooks.h"
#include "c&_memory.h" // Varsayılan bellek ayırma/serbest bırakma fonksiyonları için (veya GC)
#include "sahne64_api.h" // Gerekirse düşük seviyeli bellek işlemleri için

// Mevcut hook fonksiyonlarının statik değişkenleri
static c_ampersand_allocate_hook_t internal_allocate_hook = NULL;
static c_ampersand_free_hook_t internal_free_hook = NULL;

// Harici olarak erişilebilen mevcut hook fonksiyonları
c_ampersand_allocate_hook_t current_allocate_hook = NULL;
c_ampersand_free_hook_t current_free_hook = NULL;

// Yeni bir bellek ayırma hook fonksiyonu ayarlar.
void c_ampersand_set_allocate_hook(c_ampersand_allocate_hook_t hook) {
    internal_allocate_hook = hook;
    current_allocate_hook = hook;
}

// Mevcut bellek ayırma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_allocate_hook() {
    internal_allocate_hook = NULL;
    current_allocate_hook = NULL;
}

// Yeni bir bellek serbest bırakma hook fonksiyonu ayarlar.
void c_ampersand_set_free_hook(c_ampersand_free_hook_t hook) {
    internal_free_hook = hook;
    current_free_hook = hook;
}

// Mevcut bellek serbest bırakma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_free_hook() {
    internal_free_hook = NULL;
    current_free_hook = NULL;
}

// --- Hook'ların Kullanımı (Örnek: c&_memory.c içinde) ---

#ifdef C_AMPERSAND_MEMORY_IMPLEMENTATION // Sadece c&_memory.c içinde derlenirken

#include <stdio.h> // Örnek hook fonksiyonları için

// Örnek bir bellek ayırma hook fonksiyonu
void *debug_allocate_hook(size_t size) {
    c_ampersand_println("Bellek ayrılıyor: %zu byte", size);
    void *ptr = sahne64_memory_allocate(size); // Veya c_ampersand_gc_allocate
    c_ampersand_println("Bellek ayrıldı: %p", ptr);
    return ptr;
}

// Örnek bir bellek serbest bırakma hook fonksiyonu
void debug_free_hook(void *ptr) {
    c_ampersand_println("Bellek serbest bırakılıyor: %p", ptr);
    sahne64_memory_deallocate(ptr); // Veya ilgili serbest bırakma fonksiyonu
    c_ampersand_println("Bellek serbest bırakıldı.");
}

c_ampersand_result c_ampersand_allocate(size_t size, void **ptr) {
    if (ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    void *allocated_memory;
    if (internal_allocate_hook != NULL) {
        allocated_memory = internal_allocate_hook(size);
    } else {
        allocated_memory = sahne64_memory_allocate(size); // Veya c_ampersand_gc_allocate(size);
        if (allocated_memory == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
    }
    *ptr = allocated_memory;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_free(void *ptr) {
    if (ptr == NULL) {
        return C_AMPERSAND_OK_RESULT;
    }

    if (internal_free_hook != NULL) {
        internal_free_hook(ptr);
    } else {
        sahne64_memory_deallocate(ptr); // Veya ilgili serbest bırakma fonksiyonu
    }
    return C_AMPERSAND_OK_RESULT;
}

#endif // C_AMPERSAND_MEMORY_IMPLEMENTATION
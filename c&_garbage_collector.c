#include "c&_garbage_collector.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası
#include <stdlib.h>
#include <stdio.h>

// --- Dahili Sabitler ve Veri Yapıları ---

#define HEAP_SIZE (1024 * 1024) // Örnek heap boyutu (1MB)
static char *heap_start;
static char *heap_end;
static c_ampersand_gc_object_header *free_list = NULL;

// Kök nesnelerin listesi
#define MAX_ROOTS 128
static void **roots[MAX_ROOTS];
static int root_count = 0;

// --- Yardımcı Fonksiyonlar ---

// Belirtilen boyutta bir bellek bloğu bulur veya oluşturur.
static c_ampersand_gc_object_header *allocate_block(size_t size) {
    // Basit bir first-fit algoritması
    c_ampersand_gc_object_header *current = free_list;
    c_ampersand_gc_object_header *prev = NULL;

    while (current != NULL) {
        if (current->size >= size) {
            // Bloğu listeden çıkar
            if (prev == NULL) {
                free_list = current->next;
            } else {
                prev->next = current->next;
            }
            return current;
        }
        prev = current;
        current = current->next;
    }

    // Yeterli boş blok yoksa, heap'ten yeni bir blok ayır
    size_t total_size = sizeof(c_ampersand_gc_object_header) + size;
    if ((char *)heap_start + total_size <= heap_end) {
        c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)heap_start;
        header->size = size;
        header->marked = false;
        header->next = NULL;
        heap_start += total_size;
        return header;
    }

    return NULL; // Bellek yetersiz
}

// Bir bellek bloğunu serbest listeye ekler.
static void free_block(c_ampersand_gc_object_header *header) {
    header->next = free_list;
    free_list = header;
}

// Heap üzerindeki tüm nesnelerin işaretini kaldırır.
static void unmark_all() {
    c_ampersand_gc_object_header *current = (c_ampersand_gc_object_header *)((char *)heap_start - sizeof(c_ampersand_gc_object_header));
    while ((char *)current >= heap_start - (heap_end - (char *)heap_start) && (char *)current < heap_end) {
        current->marked = false;
        current = (c_ampersand_gc_object_header *)((char *)current - sizeof(c_ampersand_gc_object_header) - current->size);
    }
}

// Köklerden başlayarak canlı nesneleri işaretler (basit bir örnek).
static void mark_live_objects() {
    for (int i = 0; i < root_count; ++i) {
        void *ptr = *roots[i];
        if (ptr != NULL) {
            c_ampersand_gc_mark(ptr);
        }
    }
    // Burada nesneler arasındaki referansları takip ederek diğer canlı nesneler de işaretlenmelidir.
    // Bu örnekte basitlik için atlanmıştır.
}

// İşaretsiz nesneleri topla (serbest listeye ekle).
static void sweep() {
    char *current = (char *)heap_start - sizeof(c_ampersand_gc_object_header);
    while (current >= heap_start - (heap_end - (char *)heap_start) && current < (char *)heap_end) {
        c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)current;
        if (!header->marked) {
            free_block(header);
        }
        current -= (sizeof(c_ampersand_gc_object_header) + header->size);
    }
}

// --- Çöp Toplama Fonksiyonlarının Implementasyonu ---

void *c_ampersand_gc_allocate(size_t size) {
    if (heap_start == NULL) {
        // Çöp toplayıcı başlatılmamış
        return NULL;
    }
    c_ampersand_gc_object_header *header = allocate_block(size);
    if (header != NULL) {
        return (void *)((char *)header + sizeof(c_ampersand_gc_object_header));
    } else {
        // Bellek yetersiz, çöp toplamayı dene
        c_ampersand_gc_collect();
        header = allocate_block(size);
        if (header != NULL) {
            return (void *)((char *)header + sizeof(c_ampersand_gc_object_header));
        }
        return NULL; // Hala bellek yetersiz
    }
}

void c_ampersand_gc_mark(void *ptr) {
    if (ptr == NULL || heap_start == NULL) {
        return;
    }
    c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)((char *)ptr - sizeof(c_ampersand_gc_object_header));
    // İşaretleme sadece heap içindeki nesneler için yapılmalı
    if ((char *)header >= heap_start - (heap_end - (char *)heap_start) && (char *)header < heap_end) {
        header->marked = true;
        // Burada nesnenin içindeki diğer pointer'lar da işaretlenmelidir (özyinelemeli tarama).
        // Bu örnekte basitlik için atlanmıştır.
    }
}

void c_ampersand_gc_collect() {
    if (heap_start == NULL) {
        return;
    }
    c_ampersand_println("Çöp toplama başlatılıyor...");
    unmark_all();
    mark_live_objects();
    sweep();
    c_ampersand_println("Çöp toplama tamamlandı.");
}

void c_ampersand_gc_register_root(void **root_ptr) {
    if (root_count < MAX_ROOTS) {
        roots[root_count++] = root_ptr;
    } else {
        c_ampersand_println("Uyarı: Çok fazla kök nesne kaydedildi.");
    }
}

void c_ampersand_gc_unregister_root(void **root_ptr) {
    for (int i = 0; i < root_count; ++i) {
        if (roots[i] == root_ptr) {
            // Diziden çıkar
            for (int j = i; j < root_count - 1; ++j) {
                roots[j] = roots[j + 1];
            }
            root_count--;
            return;
        }
    }
    c_ampersand_println("Uyarı: Kök nesne bulunamadı.");
}

c_ampersand_result c_ampersand_gc_init() {
    if (heap_start == NULL) {
        heap_start = (char *)sahne64_memory_allocate(HEAP_SIZE);
        if (heap_start == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        heap_end = heap_start + HEAP_SIZE;
        free_list = NULL; // Başlangıçta boş
        root_count = 0;
        c_ampersand_println("Çöp toplayıcı başlatıldı (Heap boyutu: %zu)", HEAP_SIZE);
        return C_AMPERSAND_OK_RESULT;
    } else {
        c_ampersand_println("Çöp toplayıcı zaten başlatılmış.");
        return C_AMPERSAND_OK_RESULT;
    }
}

c_ampersand_result c_ampersand_gc_shutdown() {
    if (heap_start != NULL) {
        sahne64_memory_deallocate(heap_start);
        heap_start = NULL;
        heap_end = NULL;
        free_list = NULL;
        root_count = 0;
        c_ampersand_println("Çöp toplayıcı kapatıldı.");
        return C_AMPERSAND_OK_RESULT;
    } else {
        c_ampersand_println("Çöp toplayıcı zaten kapalı.");
        return C_AMPERSAND_OK_RESULT;
    }
}

// ... diğer çöp toplama ile ilgili fonksiyonların implementasyonları buraya gelebilir
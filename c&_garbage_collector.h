#ifndef C_AMPERSAND_GARBAGE_COLLECTOR_H
#define C_AMPERSAND_GARBAGE_COLLECTOR_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Çöp toplayıcı tarafından yönetilen bir nesnenin başlığı
typedef struct c_ampersand_gc_object_header {
    size_t size;
    bool marked;
    struct c_ampersand_gc_object_header *next;
} c_ampersand_gc_object_header;

// Çöp toplayıcı tarafından yönetilen bellekten belirli boyutta bir nesne ayırır.
void *c_ampersand_gc_allocate(size_t size);

// Bir nesnenin işaretini ayarlar (çöp toplama sırasında kullanılır).
void c_ampersand_gc_mark(void *ptr);

// Çöp toplama döngüsünü tetikler.
void c_ampersand_gc_collect();

// Çöp toplayıcının taraması gereken kök (root) bir nesne kaydeder.
void c_ampersand_gc_register_root(void **root_ptr);

// Kaydedilmiş bir kök nesneyi listeden çıkarır.
void c_ampersand_gc_unregister_root(void **root_ptr);

// Çöp toplayıcıyı başlatır.
c_ampersand_result c_ampersand_gc_init();

// Çöp toplayıcıyı kapatır ve kullanılan kaynakları serbest bırakır.
c_ampersand_result c_ampersand_gc_shutdown();

// ... diğer çöp toplama ile ilgili fonksiyonlar eklenebilir (örneğin, istatistikler)

#endif // C_AMPERSAND_GARBAGE_COLLECTOR_H
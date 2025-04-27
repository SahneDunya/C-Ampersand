#ifndef C_AMPERSAND_GARBAGE_COLLECTOR_H
#define C_AMPERSAND_GARBAGE_COLLECTOR_H

// Temel C& tipleri ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// SAHNE64 C API başlığı (GC heap'ini almak için)
#include "sahne.h"

#include <stddef.h> // size_t, NULL için
#include <stdbool.h> // bool için

// Çöp toplayıcı tarafından yönetilen bir nesnenin başlığı
typedef struct c_ampersand_gc_object_header {
    size_t size;             // Nesnenin boyutu (header hariç)
    bool marked;             // İşaretle fazında kullanılan bayrak
    // Diğer nesnelerin layout bilgisini tutacak alanlar eklenebilir (pointer offsetleri vb.)
    struct c_ampersand_gc_object_header *next_free; // Sadece serbest listesinde kullanılır
    // struct c_ampersand_gc_object_header *next_object; // Heap gezinmesi için gerekebilir
} c_ampersand_gc_object_header;

// Çöp toplayıcı tarafından yönetilen bellekten belirli boyutta bir nesne ayırır.
// SAHNE64 GC heap'ini kullanır. Bellek yetersizse GC tetikler.
void *c_ampersand_gc_allocate(size_t size);

// Bir nesnenin ve referans ettiği diğer nesnelerin işaretini ayarlar (çöp toplama mark fazı).
// Gerçek implementasyon, işaretlenen nesnenin tipine bakarak içindeki pointerları takip etmelidir.
void c_ampersand_gc_mark(void *ptr);

// Çöp toplama döngüsünü (işaretle ve süpür) tetikler.
void c_ampersand_gc_collect();

// Çöp toplayıcının taraması gereken kök (root) bir işaretçiyi kaydeder.
// Kök işaretçiler stack'te veya global/statik değişkenlerdeki GC nesnelerine işaret eden pointerlardır.
// Kaydedilen pointer'ın *adresini* verirsiniz (void **root_ptr).
void c_ampersand_gc_register_root(void **root_ptr);

// Kaydedilmiş bir kök işaretçiyi listeden çıkarır.
void c_ampersand_gc_unregister_root(void **root_ptr);

// Çöp toplayıcıyı başlatır.
// SAHNE64'ten büyük bir bellek bloğu ister.
c_ampersand_result c_ampersand_gc_init();

// Çöp toplayıcıyı kapatır ve SAHNE64'ten alınan büyük bellek bloğunu geri verir.
c_ampersand_result c_ampersand_gc_shutdown();

// ... diğer çöp toplama ile ilgili fonksiyonlar eklenebilir (örneğin, istatistikler, heap boyutu ayarlama)

#endif // C_AMPERSAND_GARBAGE_COLLECTOR_H

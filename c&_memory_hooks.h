#ifndef C_AMPERSAND_MEMORY_HOOKS_H
#define C_AMPERSAND_MEMORY_HOOKS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// size_t için
#include <stddef.h>


// Bellek ayırma hook fonksiyonu için tip tanımı
// Bu hook, c_ampersand_allocate'nin davranışını devralabilir.
// İmzası c_ampersand_allocate ile aynı olmalıdır.
typedef c_ampersand_result (*c_ampersand_allocate_hook_t)(size_t size, void **ptr);

// Bellek serbest bırakma hook fonksiyonu için tip tanımı
// Bu hook, c_ampersand_free'nin davranışını devralabilir.
// İmzası c_ampersand_free ile aynı olmalıdır.
typedef c_ampersand_result (*c_ampersand_free_hook_t)(void *ptr, size_t size);

// Mevcut bellek ayırma hook fonksiyonu işaretçisi.
// Bu işaretçi c&_memory_hooks.c içinde tanımlanır ve set/unset fonksiyonlarıyla değiştirilir.
extern c_ampersand_allocate_hook_t c_ampersand_allocate_hook;

// Mevcut bellek serbest bırakma hook fonksiyonu işaretçisi.
// Bu işaretçi c&_memory_hooks.c içinde tanımlanır ve set/unset fonksiyonlarıyla değiştirilir.
extern c_ampersand_free_hook_t c_ampersand_free_hook;


// Yeni bir bellek ayırma hook fonksiyonu ayarlar.
// hook: Ayarlanacak hook fonksiyonu. NULL varsayılan davranışı geri yükler.
void c_ampersand_set_allocate_hook(c_ampersand_allocate_hook_t hook);

// Mevcut bellek ayırma hook fonksiyonunu sıfırlar (varsayılan Sahne64/GC davranışını geri yükler).
void c_ampersand_unset_allocate_hook();

// Yeni bir bellek serbest bırakma hook fonksiyonu ayarlar.
// hook: Ayarlanacak hook fonksiyonu. NULL varsayılan davranışı geri yükler.
void c_ampersand_set_free_hook(c_ampersand_free_hook_t hook);

// Mevcut bellek serbest bırakma hook fonksiyonunu sıfırlar (varsayılan Sahne64/GC davranışını geri yükler).
void c_ampersand_unset_free_hook();

#endif // C_AMPERSAND_MEMORY_HOOKS_H

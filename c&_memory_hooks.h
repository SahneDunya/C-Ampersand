#ifndef C_AMPERSAND_MEMORY_HOOKS_H
#define C_AMPERSAND_MEMORY_HOOKS_H

#include "c&_stdlib.h" // Temel tipler için
#include <stddef.h> // size_t için

// Bellek ayırma hook fonksiyonu için tip tanımı
typedef void *(*c_ampersand_allocate_hook_t)(size_t size);

// Bellek serbest bırakma hook fonksiyonu için tip tanımı
typedef void (*c_ampersand_free_hook_t)(void *ptr);

// Mevcut bellek ayırma hook fonksiyonu
extern c_ampersand_allocate_hook_t current_allocate_hook;

// Mevcut bellek serbest bırakma hook fonksiyonu
extern c_ampersand_free_hook_t current_free_hook;

// Yeni bir bellek ayırma hook fonksiyonu ayarlar.
void c_ampersand_set_allocate_hook(c_ampersand_allocate_hook_t hook);

// Mevcut bellek ayırma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_allocate_hook();

// Yeni bir bellek serbest bırakma hook fonksiyonu ayarlar.
void c_ampersand_set_free_hook(c_ampersand_free_hook_t hook);

// Mevcut bellek serbest bırakma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_free_hook();

#endif // C_AMPERSAND_MEMORY_HOOKS_H
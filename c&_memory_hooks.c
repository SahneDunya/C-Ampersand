#include "c&_memory_hooks.h"

// Hook fonksiyonu işaretçilerinin tanımları
// Bu işaretçiler extern olarak bildirilmiştir ve set/unset fonksiyonları bunları değiştirir.
c_ampersand_allocate_hook_t c_ampersand_allocate_hook = NULL;
c_ampersand_free_hook_t c_ampersand_free_hook = NULL;

// --- Hook Yönetimi Fonksiyonları ---

// Yeni bir bellek ayırma hook fonksiyonu ayarlar.
void c_ampersand_set_allocate_hook(c_ampersand_allocate_hook_t hook) {
    c_ampersand_allocate_hook = hook; // Global hook işaretçisini ayarla
}

// Mevcut bellek ayırma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_allocate_hook() {
    c_ampersand_allocate_hook = NULL; // Global hook işaretçisini NULL yap
}

// Yeni bir bellek serbest bırakma hook fonksiyonu ayarlar.
void c_ampersand_set_free_hook(c_ampersand_free_hook_t hook) {
    c_ampersand_free_hook = hook; // Global hook işaretçisini ayarla
}

// Mevcut bellek serbest bırakma hook fonksiyonunu sıfırlar (varsayılan davranışı geri yükler).
void c_ampersand_unset_free_hook() {
    c_ampersand_free_hook = NULL; // Global hook işaretçisini NULL yap
}

// NOT: Hook'ların nasıl kullanıldığına dair implementasyon artık c&_memory.c içinde olacaktır.
// Örnek debug hook fonksiyonları ve #ifdef bloğu bu dosyadan kaldırılmıştır.

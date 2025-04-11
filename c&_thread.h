#ifndef C_AMPERSAND_THREAD_H
#define C_AMPERSAND_THREAD_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için

// Thread ID'si
typedef sahne64_thread_id_t c_ampersand_thread_id;

// Thread giriş noktası için fonksiyon pointer tipi
typedef void (*c_ampersand_thread_entry_point)(void *);

// Yeni bir thread oluşturur.
c_ampersand_result c_ampersand_thread_create(c_ampersand_thread_entry_point entry_point, size_t stack_size, void *arg, c_ampersand_thread_id *thread_id);

// Mevcut thread'i sonlandırır.
void c_ampersand_thread_exit(int code);

// ... diğer thread yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, thread'e katılma, ID alma vb.)

#endif // C_AMPERSAND_THREAD_H
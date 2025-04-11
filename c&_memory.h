#ifndef C_AMPERSAND_MEMORY_H
#define C_AMPERSAND_MEMORY_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için

// Belirli boyutta bellek ayırır.
c_ampersand_result c_ampersand_allocate(size_t size, void **ptr);

// Ayrılmış olan belleği serbest bırakır. (Otomatik bellek yönetimi olsa da gerekebilir)
c_ampersand_result c_ampersand_free(void *ptr);

// ... diğer bellek yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, yeniden boyutlandırma)

#endif // C_AMPERSAND_MEMORY_H
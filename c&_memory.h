#ifndef C_AMPERSAND_MEMORY_H
#define C_AMPERSAND_MEMORY_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Bellek yönetimi fonksiyonları ve hata tipleri için)
#include "sahne.h"

#include <stddef.h> // size_t, NULL için

// Belirli boyutta bellek ayırır.
// Bellek Sahne64 API'si kullanılarak tahsis edilir.
// size: Tahsis edilecek bellek boyutu (byte).
// ptr: Tahsis edilen belleğin adresinin yazılacağı void** işaretçisi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_allocate(size_t size, void **ptr);

// Daha önce c_ampersand_allocate ile ayrılmış olan belleği serbest bırakır.
// Bellek Sahne64 API'si kullanılarak serbest bırakılır.
// ptr: Serbest bırakılacak bellek bloğunun adresi.
// size: Serbest bırakılacak bellek bloğunun boyutu (tahsis edilen boyutla aynı olmalı).
// Dönüş: Başarı veya hata belirten c_ampersand_result.
// NOT: Çöp toplama (GC) kullanıldığında bu fonksiyonun nasıl entegre edileceği
// C& dilinin bellek modeline bağlıdır (örn. GC dışı manuel yönetim için kullanılabilir).
c_ampersand_result c_ampersand_free(void *ptr, size_t size);

// ... diğer bellek yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, yeniden boyutlandırma)

#endif // C_AMPERSAND_MEMORY_H

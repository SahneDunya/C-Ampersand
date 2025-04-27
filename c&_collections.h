#ifndef C_AMPERSAND_COLLECTIONS_H
#define C_AMPERSAND_COLLECTIONS_H

// Temel C& tipleri ve hata yönetimi için (SAHNE64 C API'si de buraya dahil edilebilir)
// Varsayım: c&_stdlib.h içinde sahne.h de include ediliyor veya burada direkt include ediyoruz.
// En iyisi Sahne64 C API başlığını doğrudan dahil etmek.
#include "c&_stdlib.h" // Varsayım: c_ampersand_result, C_AMPERSAND_* hataları burada tanımlı
#include "sahne.h"     // SAHNE64 C API başlığı

#include <stddef.h> // size_t için
#include <stdbool.h>

// Dinamik dizi (Vector) veri yapısı
typedef struct {
    void *data;             // Elemanların tutulduğu bellek bölgesi (SAHNE64'ten alındı)
    size_t size;            // Şu anki eleman sayısı
    size_t capacity;        // Ayrılmış olan toplam eleman kapasitesi
    size_t element_size;    // Her bir elemanın boyutu
    size_t allocated_bytes; // SAHNE64'ten ayrılmış toplam byte boyutu (release için gerekli)
} c_ampersand_vector;

// Başarılı bir c_ampersand_result değeri
extern const c_ampersand_result C_AMPERSAND_OK_RESULT;

// Yeni bir dinamik dizi oluşturur.
// SAHNE64 bellek tahsisini kullanır.
c_ampersand_result c_ampersand_vector_create(size_t initial_capacity, size_t element_size, c_ampersand_vector *vector);

// Dinamik dizinin sonuna bir eleman ekler.
// Gerekirse SAHNE64 bellek tahsisini kullanarak boyutu artırır.
c_ampersand_result c_ampersand_vector_push(c_ampersand_vector *vector, const void *element);

// Dinamik diziden son elemanı çıkarır ve döndürür. Dizi boşsa hata döner.
c_ampersand_result c_ampersand_vector_pop(c_ampersand_vector *vector, void *element);

// Belirtilen indeksteki elemanı döndürür. Geçersiz indeks durumunda hata döner.
c_ampersand_result c_ampersand_vector_get(const c_ampersand_vector *vector, size_t index, void *element);

// Belirtilen indeksteki elemanı yeni bir değerle değiştirir. Geçersiz indeks durumunda hata döner.
c_ampersand_result c_ampersand_vector_set(c_ampersand_vector *vector, size_t index, const void *element);

// Dinamik dizideki eleman sayısını döndürür.
size_t c_ampersand_vector_size(const c_ampersand_vector *vector);

// Dinamik dizinin şu anki kapasitesini (eleman sayısı olarak) döndürür.
size_t c_ampersand_vector_capacity(const c_ampersand_vector *vector);

// Dinamik dizinin SAHNE64'ten kullandığı belleği serbest bırakır.
c_ampersand_result c_ampersand_vector_destroy(c_ampersand_vector *vector);

// ... diğer dinamik dizi operasyonları eklenebilir (örneğin, indeks ekleme, çıkarma, sıralama vb.)

#endif // C_AMPERSAND_COLLECTIONS_H

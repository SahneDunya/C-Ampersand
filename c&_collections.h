#ifndef C_AMPERSAND_COLLECTIONS_H
#define C_AMPERSAND_COLLECTIONS_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Dinamik dizi (Vector) veri yapısı
typedef struct {
    void *data;     // Elemanların tutulduğu bellek bölgesi
    size_t size;     // Şu anki eleman sayısı
    size_t capacity; // Ayrılmış olan toplam bellek kapasitesi (eleman sayısı olarak)
    size_t element_size; // Her bir elemanın boyutu
} c_ampersand_vector;

// Yeni bir dinamik dizi oluşturur.
c_ampersand_result c_ampersand_vector_create(size_t initial_capacity, size_t element_size, c_ampersand_vector *vector);

// Dinamik dizinin sonuna bir eleman ekler.
c_ampersand_result c_ampersand_vector_push(c_ampersand_vector *vector, const void *element);

// Dinamik diziden son elemanı çıkarır ve döndürür. Dizi boşsa hata döner.
c_ampersand_result c_ampersand_vector_pop(c_ampersand_vector *vector, void *element);

// Belirtilen indeksteki elemanı döndürür. Geçersiz indeks durumunda hata döner.
c_ampersand_result c_ampersand_vector_get(const c_ampersand_vector *vector, size_t index, void *element);

// Belirtilen indeksteki elemanı yeni bir değerle değiştirir. Geçersiz indeks durumunda hata döner.
c_ampersand_result c_ampersand_vector_set(c_ampersand_vector *vector, size_t index, const void *element);

// Dinamik dizideki eleman sayısını döndürür.
size_t c_ampersand_vector_size(const c_ampersand_vector *vector);

// Dinamik dizinin şu anki kapasitesini (ayrılmış bellek) döndürür.
size_t c_ampersand_vector_capacity(const c_ampersand_vector *vector);

// Dinamik dizinin kullandığı belleği serbest bırakır.
c_ampersand_result c_ampersand_vector_destroy(c_ampersand_vector *vector);

// ... diğer dinamik dizi operasyonları eklenebilir (örneğin, indeks ekleme, çıkarma, sıralama vb.)

#endif // C_AMPERSAND_COLLECTIONS_H
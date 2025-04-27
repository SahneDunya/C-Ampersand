#ifndef C_AMPERSAND_STRING_H
#define C_AMPERSAND_STRING_H

// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h"
// c_ampersand_allocate/free için
#include "c&_memory.h" // String fonksiyonları bellek tahsisi yapabilir.

#include <stddef.h> // size_t için


// --- Düşük Seviye String Yardımcı Fonksiyonları ---
// Bu fonksiyonlar C-style null sonlandırıcılı char* üzerinde işlem yapar.
// Caller, buffer tahsisinden ve yönetiminden (free) sorumludur.
// Daha yüksek seviye C& String tipi (handle veya struct) ve metotları ayrı implemente edilmelidir.


// String uzunluğunu döndürür (null sonlandırıcı hariç).
size_t c_ampersand_string_length(const char *str);


// Kaynak string'i hedef buffer'a kopyalar.
// dest: Kopyanın yazılacağı hedef buffer. c_ampersand_allocate ile ayrılmış olmalıdır.
// src: Kopyalanacak kaynak string.
// Dönüş: Başarı veya hata (geçersiz argüman). Hedef buffer'ın yeterli boyutta olması ÇAĞIRANIN sorumluluğundadır.
c_ampersand_result c_ampersand_string_copy(char *dest, const char *src);


// İki string'i sözlük sırasına göre karşılaştırır.
// Eşitse 0, s1 < s2 ise negatif, s1 > s2 ise pozitif bir değer döndürür.
int c_ampersand_string_compare(const char *s1, const char *s2);


// İki string'i birleştirir ve sonucu hedef buffer'a yazar.
// dest: Birleştirilmiş stringin yazılacağı hedef buffer. c_ampersand_allocate ile ayrılmış olmalıdır.
// s1: Birinci string.
// s2: İkinci string.
// Dönüş: Başarı veya hata (geçersiz argüman). Hedef buffer'ın yeterli boyutta olması ÇAĞIRANIN sorumluluğundadır.
c_ampersand_result c_ampersand_string_concatenate(char *dest, const char *s1, const char *s2);


// Bir string'in belirli bir alt string'ini oluşturur ve hedef buffer'a kopyalar.
// dest: Alt stringin yazılacağı hedef buffer. c_ampersand_allocate ile ayrılmış olmalıdır (length + 1 byte).
// src: Kaynak string.
// start: Alt stringin başlayacağı index (0 tabanlı).
// length: Alt stringin uzunluğu.
// Dönüş: Başarı veya hata (geçersiz argüman, start+length > src uzunluğu).
c_ampersand_result c_ampersand_string_substring(char *dest, const char *src, size_t start, size_t length);


// Bir string'in belirli bir karakteri içerip içermediğini kontrol eder.
bool c_ampersand_string_contains_char(const char *str, char c);

// Bir string'in belirli bir alt string'i içerip içermediğini kontrol eder.
bool c_ampersand_string_contains_string(const char *str, const char *substring);


// Bir string'in belirli bir karakterle başlayıp başlamadığını kontrol eder.
bool c_ampersand_string_starts_with(const char *str, char c);

// Bir string'in belirli bir alt string ile başlayıp başlamadığını kontrol eder.
bool c_ampersand_string_starts_with_string(const char *str, const char *prefix);


// Bir string'in belirli bir karakterle bitip bitmediğini kontrol eder.
bool c_ampersand_string_ends_with(const char *str, char c);

// Bir string'in belirli bir alt string ile bitip bitmediğini kontrol eder.
bool c_ampersand_string_ends_with_string(const char *str, const char *suffix);


// Bir string içindeki bir karakterin ilk konumunu bulur (0 tabanlı index). Bulunamazsa -1 döner.
int c_ampersand_string_index_of_char(const char *str, char c);

// Bir string içindeki bir alt string'in ilk konumunu bulur (0 tabanlı index). Bulunamazsa -1 döner.
int c_ampersand_string_index_of_string(const char *str, const char *substring);


// Bir tam sayıyı string'e çevirir ve sonucu hedef buffer'a yazar.
// dest: Stringin yazılacağı hedef buffer. c_ampersand_allocate ile ayrılmış olmalıdır.
// value: Stringe çevrilecek tam sayı.
// Dönüş: Başarı veya hata (geçersiz argüman, formatlama hatası). Hedef buffer'ın yeterli boyutta olması ÇAĞIRANIN sorumluluğundadır.
c_ampersand_result c_ampersand_string_from_int(char *dest, int value);


// Bir string'i tam sayıya çevirir.
// str: Tam sayıya çevrilecek string.
// out_value: Çevrilen tam sayının yazılacağı pointer.
// Dönüş: Başarı veya hata (geçersiz argüman, format hatası). Başarısız olursa out_value değiştirilmeyebilir.
c_ampersand_result c_ampersand_string_to_int(const char *str, int *out_value);


// ... diğer string fonksiyonları eklenebilir (örneğin, trim, replace, split, join)

#endif // C_AMPERSAND_STRING_H

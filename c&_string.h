#ifndef C_AMPERSAND_STRING_H
#define C_AMPERSAND_STRING_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için

// String uzunluğunu döndürür.
size_t c_ampersand_string_length(const char *str);

// Kaynak string'i hedef string'e kopyalar.
// Dikkat: Hedef buffer'ın yeterince büyük olduğundan emin olunmalıdır.
c_ampersand_result c_ampersand_string_copy(char *dest, const char *src);

// İki string'i karşılaştırır.
// Eşitse 0, s1 < s2 ise negatif, s1 > s2 ise pozitif bir değer döndürür.
int c_ampersand_string_compare(const char *s1, const char *s2);

// İki string'i birleştirir.
// Dikkat: Hedef buffer'ın yeterince büyük olduğundan emin olunmalıdır.
c_ampersand_result c_ampersand_string_concatenate(char *dest, const char *s1, const char *s2);

// Bir string'in alt string'ini oluşturur.
// Dikkat: Hedef buffer'ın yeterince büyük olduğundan emin olunmalıdır.
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

// Bir string içindeki bir karakterin ilk konumunu bulur. Bulunamazsa -1 döner.
int c_ampersand_string_index_of_char(const char *str, char c);

// Bir string içindeki bir alt string'in ilk konumunu bulur. Bulunamazsa -1 döner.
int c_ampersand_string_index_of_string(const char *str, const char *substring);

// Bir tam sayıyı string'e çevirir.
// Dikkat: Hedef buffer'ın yeterince büyük olduğundan emin olunmalıdır.
c_ampersand_result c_ampersand_string_from_int(char *dest, int value);

// Bir string'i tam sayıya çevirir. Başarısız olursa 0 döner (daha gelişmiş hata yönetimi düşünülebilir).
int c_ampersand_string_to_int(const char *str);

// ... diğer string fonksiyonları eklenebilir (örneğin, formatlama, bölme, birleştirme vb.)

#endif // C_AMPERSAND_STRING_H
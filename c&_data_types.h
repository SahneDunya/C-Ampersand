#ifndef C_AMPERSAND_DATA_TYPES_H
#define C_AMPERSAND_DATA_TYPES_H

// Temel C& tipleri ve hata yönetimi için
#include "c&_stdlib.h"
// C& içindeki dil seviyesi tip tanımları (TYPE_INT8 vb.) için
#include "c&_types.h"
// Sahne64 bellek yönetimi için gerekli olabilir, bu yüzden sahne.h'yi ekliyoruz.
#include "sahne.h"

#include <stdint.h> // Sabit genişlikli tamsayılar (int8_t, uint64_t vb.) için
#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// C Ampersand değerlerini temsil eden yapı
typedef struct c_ampersand_value {
    c_ampersand_type type; // Değerin tipi (c&_types.h'de tanımlı varsayım)
    union {
        bool bool_val;
        int8_t i8_val;
        int16_t i16_val;
        int32_t i32_val;
        int64_t i64_val;
        uint8_t u8_val;
        uint16_t u16_val;
        uint32_t u32_val;
        uint64_t u64_val;
        float float32_val;
        double float64_val;
        // String değeri için pointer ve boyutunu saklayacak bir struct kullanıyoruz
        struct {
            char *ptr;
            size_t size; // Sahne64 bellek serbest bırakma için ayrılan boyut
        } string_val;
        void *pointer_val;
        // ... diğer tipler için değerler eklenebilir (örn. handle_val)
    } data;
} c_ampersand_value;

// Başarılı bir c_ampersand_result değeri (c&_stdlib.h'de extern olarak tanımlı varsayım)
extern const c_ampersand_result C_AMPERSAND_OK_RESULT;
// Hata kodu sabitleri (c&_stdlib.h'de tanımlı varsayım)
// extern const int C_AMPERSAND_ERROR_INVALID_ARGUMENT;
// extern const int C_AMPERSAND_ERROR_OUT_OF_MEMORY;
// extern const int C_AMPERSAND_ERROR_TYPE_MISMATCH; // Kontrol yapılarından geldi

// c&_types.h içinde tanımlanan harici değişkenlerin bildirimi (varsayım: bu sabitler c&_types.h'de extern)
extern const c_ampersand_type c_ampersand_type_void;
extern const c_ampersand_type c_ampersand_type_bool;
extern const c_ampersand_type c_ampersand_type_int8;
extern const c_ampersand_type c_ampersand_type_int16;
extern const c_ampersand_type c_ampersand_type_int32;
extern const c_ampersand_type c_ampersand_type_int64;
extern const c_ampersand_type c_ampersand_type_uint8;
extern const c_ampersand_type c_ampersand_type_uint16;
extern const c_ampersand_type c_ampersand_type_uint32;
extern const c_ampersand_type c_ampersand_type_uint64;
extern const c_ampersand_type c_ampersand_type_float32;
extern const c_ampersand_type c_ampersand_type_float64;
extern const c_ampersand_type c_ampersand_type_string;
extern const c_ampersand_type c_ampersand_type_pointer;
// Varsayım: TYPE_* enum'ı c&_types.h'de tanımlı


// Bir C Ampersand tipini string olarak döndürür (hata ayıklama için).
const char *c_ampersand_type_to_string(c_ampersand_type type);


// Yeni bir C Ampersand değeri oluşturur.
// out_value: Oluşturulan değerin yazılacağı pointer.
c_ampersand_result c_ampersand_value_create(c_ampersand_type type, c_ampersand_value *out_value);

// Bir C Ampersand değerini serbest bırakır (özellikle heap'te ayrılan string ve yönetilen pointerlar için).
// value: Serbest bırakılacak değerin pointer'ı.
void c_ampersand_value_free(c_ampersand_value *value);

// Bir C Ampersand değerinin içeriğini ayarlar.
// value: Ayarlanacak değerin pointer'ı.
// val: Ayarlanacak ham değer.
c_ampersand_result c_ampersand_value_set_bool(c_ampersand_value *value, bool val);
c_ampersand_result c_ampersand_value_set_int(c_ampersand_value *value, int64_t val);
c_ampersand_result c_ampersand_value_set_uint(c_ampersand_value *value, uint64_t val);
c_ampersand_result c_ampersand_value_set_float(c_ampersand_value *value, double val);
// String değeri ayarlar. Giriş stringinin kopyasını heap'te Sahne64 belleğini kullanarak oluşturur.
c_ampersand_result c_ampersand_value_set_string(c_ampersand_value *value, const char *val);
c_ampersand_result c_ampersand_value_set_pointer(c_ampersand_value *value, void *val);

// Bir C Ampersand değerinin içeriğini alır.
// value: İçeriği alınacak değerin pointer'ı.
bool c_ampersand_value_get_bool(const c_ampersand_value *value);
int64_t c_ampersand_value_get_int(const c_ampersand_value *value);
uint64_t c_ampersand_value_get_uint(const c_ampersand_value *value);
double c_ampersand_value_get_float(const c_ampersand_value *value);
// String değerini alır. Dikkat: Döndürülen pointer yönetilen bellektedir, free edilmemelidir.
const char *c_ampersand_value_get_string(const c_ampersand_value *value);
void *c_ampersand_value_get_pointer(const c_ampersand_value *value);

// İki C Ampersand değerinin eşit olup olmadığını kontrol eder.
// val1, val2: Karşılaştırılacak değerlerin pointerları.
bool c_ampersand_value_equals(const c_ampersand_value *val1, const c_ampersand_value *val2);

// ... diğer veri tipi yönetimi ile ilgili fonksiyonlar eklenebilir (kopyalama, deep copy vb.)

#endif // C_AMPERSAND_DATA_TYPES_H

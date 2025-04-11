#ifndef C_AMPERSAND_DATA_TYPES_H
#define C_AMPERSAND_DATA_TYPES_H

#include "c&_stdlib.h"
#include "c&_types.h"
#include <stdint.h>
#include <stddef.h>

// C Ampersand değerlerini temsil eden yapı
typedef struct c_ampersand_value {
    c_ampersand_type type;
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
        char *string_val;
        void *pointer_val;
        // ... diğer tipler için değerler eklenebilir
    } data;
} c_ampersand_value;

// Yeni bir C Ampersand değeri oluşturur.
c_ampersand_result c_ampersand_value_create(c_ampersand_type type, c_ampersand_value *out_value);

// Bir C Ampersand değerini serbest bırakır (özellikle string ve pointer için).
void c_ampersand_value_free(c_ampersand_value *value);

// Bir C Ampersand değerinin içeriğini ayarlar.
c_ampersand_result c_ampersand_value_set_bool(c_ampersand_value *value, bool val);
c_ampersand_result c_ampersand_value_set_int(c_ampersand_value *value, int64_t val);
c_ampersand_result c_ampersand_value_set_uint(c_ampersand_value *value, uint64_t val);
c_ampersand_result c_ampersand_value_set_float(c_ampersand_value *value, double val);
c_ampersand_result c_ampersand_value_set_string(c_ampersand_value *value, const char *val);
c_ampersand_result c_ampersand_value_set_pointer(c_ampersand_value *value, void *val);

// Bir C Ampersand değerinin içeriğini alır.
bool c_ampersand_value_get_bool(const c_ampersand_value *value);
int64_t c_ampersand_value_get_int(const c_ampersand_value *value);
uint64_t c_ampersand_value_get_uint(const c_ampersand_value *value);
double c_ampersand_value_get_float(const c_ampersand_value *value);
const char *c_ampersand_value_get_string(const c_ampersand_value *value);
void *c_ampersand_value_get_pointer(const c_ampersand_value *value);

// İki C Ampersand değerinin eşit olup olmadığını kontrol eder.
bool c_ampersand_value_equals(const c_ampersand_value *val1, const c_ampersand_value *val2);

// ... diğer veri tipi yönetimi ile ilgili fonksiyonlar eklenebilir

#endif // C_AMPERSAND_DATA_TYPES_H
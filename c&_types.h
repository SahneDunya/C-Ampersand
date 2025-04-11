#ifndef C_AMPERSAND_TYPES_H
#define C_AMPERSAND_TYPES_H

#include "c&_stdlib.h" // Temel tipler için
#include <stdbool.h>

// C Ampersand dilinin temel veri tipleri
typedef enum {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
    TYPE_STRING,
    TYPE_POINTER,
    // ... diğer tipler (diziler, yapılar vb.) eklenebilir
    TYPE_COUNT
} c_ampersand_type_kind;

// Bir türü temsil eden yapı
typedef struct c_ampersand_type {
    c_ampersand_type_kind kind;
    // ... türle ilgili ek bilgiler (örneğin, pointer için işaret ettiği tür, dizi için boyut) eklenebilir
} c_ampersand_type;

// Temel türler için yardımcı tanımlamalar
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

// Türlerin isimlerini döndüren fonksiyon (debug amaçlı)
const char *c_ampersand_type_to_string(c_ampersand_type type);

#endif // C_AMPERSAND_TYPES_H
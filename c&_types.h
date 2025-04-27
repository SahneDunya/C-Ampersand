#ifndef C_AMPERSAND_TYPES_H
#define C_AMPERSAND_TYPES_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// bool için (c&_stdlib.h'de de olabilir)
#include <stdbool.h>
// size_t için (c&_stdlib.h'de de olabilir)
#include <stddef.h>

// C Ampersand dilinin temel veri tipleri enum'ı
typedef enum {
    TYPE_UNKNOWN = 0, // Bilinmeyen veya henüz belirlenmemiş tip
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
    TYPE_ARRAY, // Dizi
    TYPE_STRUCT, // Yapı
    TYPE_FUNCTION, // Fonksiyon tipi

    // ... diğer tipler (enum, union, referans, dilim vb.) eklenebilir
    TYPE_COUNT // Tüm temel type kind'larının sayısı
} c_ampersand_type_kind;

// Bir türü temsil eden yapı
typedef struct c_ampersand_type {
    c_ampersand_type_kind kind; // Temel tür türü

    // Türle ilgili ek bilgiler (Union veya ayrı alanlar olarak)
    // Örneğin:
    union {
        // Pointer için: İşaret ettiği tür
        struct {
            struct c_ampersand_type *target_type; // Heap'te olabilir veya pointer'a işaret edebilir
        } pointer_info;

        // Dizi için: Eleman türü ve boyut
        struct {
            struct c_ampersand_type *element_type; // Heap'te olabilir veya pointer'a işaret edebilir
            size_t array_size; // Sabit boyutlu diziler için
            // Dinamik diziler/dilimler için başka alanlar gerekebilir
        } array_info;

        // Yapı (Struct) için: Adı, üyeleri
        struct {
            const char *name; // Struct adı (string sabiti veya heap'te)
            size_t name_allocated_size; // Eğer heap'teyse
            // Yapı üyeleri listesi veya lookup tablosu (ayrı tanımlanabilir)
            // struct c_ampersand_struct_member *members;
            // size_t num_members;
        } struct_info;

        // Fonksiyon tipi için: Parametre tipleri, dönüş tipi
        struct {
            struct c_ampersand_type *return_type; // Dönüş tipi (pointer)
            // Parametre tipleri listesi (ayrı bir dizi veya bağlı liste)
            // struct c_ampersand_type *parameter_types; // Dizi başı
            // size_t num_parameters;
        } function_info;

    } info;

    // Eğer struct/array/function type nesneleri heap'te yaratılacaksa,
    // bu struct'ın kendisi için allocate boyutu bilgisi eklenebilir.
    // size_t type_allocated_size;

} c_ampersand_type;

// Temel sabit türler için extern tanımlamalar
// Bunlar c&_types.c'de tanımlanır.
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
extern const c_ampersand_type c_ampersand_type_unknown;


// Tür sistemini başlatan fonksiyon (singleton temel türleri tanımlar).
// c&_types.c'de implemente edilir.
c_ampersand_result c_ampersand_types_init();

// Tür sistemini kapatan fonksiyon (varsa ayrılan belleği serbest bırakır).
// c&_types.c'de implemente edilir.
c_ampersand_result c_ampersand_types_shutdown();


// Türlerin isimlerini döndüren fonksiyon (debug amaçlı).
// c&_types.c'de implemente edilir.
const char *c_ampersand_type_to_string(c_ampersand_type type);

// İki türün tamamen eşit olup olmadığını kontrol eder.
// c&_types.c'de implemente edilir.
bool c_ampersand_type_equals(c_ampersand_type t1, c_ampersand_type t2);

// Bir türün diğerine atanabilir/uyumlu olup olmadığını kontrol eder (örn. int -> float).
// c&_types.c'de implemente edilir.
bool c_ampersand_type_is_compatible(c_ampersand_type from, c_ampersand_type to);

// ... Diğer tür yönetimi fonksiyonları (pointer türü oluşturma, dizi türü oluşturma vb.)

#endif // C_AMPERSAND_TYPES_H

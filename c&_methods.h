#ifndef C_AMPERSAND_METHODS_H
#define C_AMPERSAND_METHODS_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_types.h"       // Veri tipleri için
#include "c&_data_types.h" // Değerleri tutmak için

// Bir metot implementasyonu için fonksiyon pointer tipi
typedef c_ampersand_result (*c_ampersand_method_func)(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);

// Bir metodu temsil eden yapı
typedef struct c_ampersand_method {
    const char *name;
    c_ampersand_type_kind belongs_to_type;
    c_ampersand_method_func func_ptr;
    struct c_ampersand_method *next; // Aynı tipteki metotlar için bağlı liste
} c_ampersand_method;

// Bir tipin metot tablosunu temsil eden yapı
typedef struct c_ampersand_method_table {
    c_ampersand_type_kind type;
    c_ampersand_method *methods;
    struct c_ampersand_method_table *next; // Tüm metot tabloları için bağlı liste
} c_ampersand_method_table;

// Tüm metot tablolarının başı
extern c_ampersand_method_table *global_method_table;

// Yeni bir metot oluşturur.
c_ampersand_method *c_ampersand_method_create(const char *name, c_ampersand_type_kind type, c_ampersand_method_func func_ptr);

// Bir metodu serbest bırakır.
void c_ampersand_method_free(c_ampersand_method *method);

// Belirli bir tipe bir metot ekler.
c_ampersand_result c_ampersand_method_register(c_ampersand_type_kind type, c_ampersand_method *method);

// Bir nesne ve metot adına göre metodu arar.
c_ampersand_method *c_ampersand_method_lookup(c_ampersand_type_kind object_type, const char *method_name);

// Bir nesne üzerinde bir metodu çağırır.
c_ampersand_result c_ampersand_method_call(c_ampersand_value *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);

// Metot sistemini başlatır.
c_ampersand_result c_ampersand_methods_init();

// Metot sistemini kapatır.
c_ampersand_result c_ampersand_methods_shutdown();

#endif // C_AMPERSAND_METHODS_H
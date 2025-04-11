#ifndef C_AMPERSAND_OBJECT_H
#define C_AMPERSAND_OBJECT_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_types.h"       // Veri tipleri için
#include "c&_data_types.h" // Değerleri tutmak için
#include "c&_methods.h"    // Metotlar için

// Bir nesneyi temsil eden yapı
typedef struct c_ampersand_object {
    c_ampersand_type type;
    void *data; // Nesnenin verisine işaretçi
} c_ampersand_object;

// Yeni bir nesne oluşturur.
c_ampersand_result c_ampersand_object_create(c_ampersand_type type, c_ampersand_object **out_object);

// Bir nesneyi serbest bırakır.
void c_ampersand_object_free(c_ampersand_object *object);

// Bir nesnenin bir alanının değerini alır.
c_ampersand_result c_ampersand_object_get_field(const c_ampersand_object *object, const char *field_name, c_ampersand_value *out_value);

// Bir nesnenin bir alanının değerini ayarlar.
c_ampersand_result c_ampersand_object_set_field(c_ampersand_object *object, const char *field_name, const c_ampersand_value *value);

// Bir nesne üzerinde bir metodu çağırır (dolaylı olarak c&_methods.h'yi kullanır).
c_ampersand_result c_ampersand_object_call_method(c_ampersand_object *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);

#endif // C_AMPERSAND_OBJECT_H
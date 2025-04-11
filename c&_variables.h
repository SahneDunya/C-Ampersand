#ifndef C_AMPERSAND_VARIABLES_H
#define C_AMPERSAND_VARIABLES_H

#include "c&_stdlib.h" // Temel tipler için
#include "c&_types.h"   // Veri tipleri için
#include "c&_data_types.h" // Değerleri tutmak için
#include <stdbool.h>

// Bir değişkeni temsil eden yapı
typedef struct c_ampersand_variable {
    char *name;
    c_ampersand_type type;
    c_ampersand_value value;
    // ... diğer değişken özellikleri (scope vb.) eklenebilir
    struct c_ampersand_variable *next; // Aynı scope içindeki bir sonraki değişken için (basit liste)
} c_ampersand_variable;

// Bir scope'u (kapsamı) temsil eden yapı
typedef struct c_ampersand_scope {
    c_ampersand_variable *variables; // Bu scope'taki değişkenlerin listesinin başı
    struct c_ampersand_scope *parent;   // Üst scope (eğer varsa)
} c_ampersand_scope;

// Global scope (kök scope)
extern c_ampersand_scope *global_scope;

// Yeni bir değişken oluşturur.
c_ampersand_variable *c_ampersand_variable_create(const char *name, c_ampersand_type type);

// Bir değişkeni serbest bırakır.
void c_ampersand_variable_free(c_ampersand_variable *var);

// Bir değişkene değer atar.
c_ampersand_result c_ampersand_variable_set_value(c_ampersand_variable *var, const c_ampersand_value *value);

// Bir değişkenin değerini alır.
c_ampersand_result c_ampersand_variable_get_value(const c_ampersand_variable *var, c_ampersand_value *out_value);

// Geçerli scope'ta belirli bir ada sahip bir değişkeni arar.
c_ampersand_variable *c_ampersand_variable_lookup(c_ampersand_scope *scope, const char *name);

// Yeni bir scope oluşturur.
c_ampersand_scope *c_ampersand_scope_create(c_ampersand_scope *parent);

// Bir scope'u serbest bırakır (içindeki değişkenleri de serbest bırakır).
void c_ampersand_scope_free(c_ampersand_scope *scope);

// Geçerli scope'a bir değişken ekler.
c_ampersand_result c_ampersand_scope_add_variable(c_ampersand_scope *scope, c_ampersand_variable *var);

// ... diğer değişken ve scope yönetimi ile ilgili fonksiyonlar eklenebilir

#endif // C_AMPERSAND_VARIABLES_H
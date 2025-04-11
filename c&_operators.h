#ifndef C_AMPERSAND_OPERATORS_H
#define C_AMPERSAND_OPERATORS_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_types.h"       // Veri tipleri için
#include "c&_data_types.h" // Değerleri tutmak için

// Desteklenen operatör türleri
typedef enum {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS_THAN,
    OPERATOR_GREATER_THAN,
    // ... diğer operatörler eklenebilir
    OPERATOR_COUNT
} c_ampersand_operator_type;

// İki operand üzerinde işlem yapan bir operatör fonksiyonu için tip tanımı
typedef c_ampersand_result (*c_ampersand_binary_operator_func)(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result);

// Tek bir operand üzerinde işlem yapan bir operatör fonksiyonu için tip tanımı (gerekirse)
typedef c_ampersand_result (*c_ampersand_unary_operator_func)(const c_ampersand_value *operand, c_ampersand_value *result);

// Belirli bir operatör türü için operatör fonksiyonunu döndürür.
c_ampersand_binary_operator_func c_ampersand_operator_get_binary_function(c_ampersand_operator_type type);

// ... operatör önceliği ve associativity ile ilgili tanımlamalar eklenebilir

#endif // C_AMPERSAND_OPERATORS_H
#ifndef C_AMPERSAND_OPERATORS_H
#define C_AMPERSAND_OPERATORS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// C& veri tipleri için (c_ampersand_type, c_ampersand_type_kind)
#include "c&_types.h"
// C& değer yapıları ve value_equals gibi yardımcı fonksiyonlar için (c_ampersand_value, c_ampersand_value_equals)
#include "c&_data_types.h"

// Desteklenen ikili operatör türleri
typedef enum {
    OPERATOR_ADD,         // +
    OPERATOR_SUBTRACT,    // -
    OPERATOR_MULTIPLY,    // *
    OPERATOR_DIVIDE,      // /
    OPERATOR_EQUAL,       // ==
    OPERATOR_NOT_EQUAL,   // !=
    OPERATOR_LESS_THAN,   // <
    OPERATOR_GREATER_THAN,// >
    // ... diğer operatörler eklenebilir (mantıksal, bitwise, atama vb.)
    OPERATOR_COUNT        // Tanımlanan operatör sayısı
} c_ampersand_operator_type;

// İki operand üzerinde işlem yapan bir operatör fonksiyonu için tip tanımı
// operand1: İlk operand (c_ampersand_value pointer'ı)
// operand2: İkinci operand (c_ampersand_value pointer'ı)
// result: İşlem sonucunun yazılacağı c_ampersand_value pointer'ı
// Dönüş: Başarı veya hata belirten c_ampersand_result.
typedef c_ampersand_result (*c_ampersand_binary_operator_func)(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result);

// Tek bir operand üzerinde işlem yapan bir operatör fonksiyonu için tip tanımı (gerekirse)
// operand: İşlem yapılacak operand (c_ampersand_value pointer'ı)
// result: İşlem sonucunun yazılacağı c_ampersand_value pointer'ı
// Dönüş: Başarı veya hata belirten c_ampersand_result.
typedef c_ampersand_result (*c_ampersand_unary_operator_func)(const c_ampersand_value *operand, c_ampersand_value *result);

// Belirli bir ikili operatör türü için implementasyon fonksiyonunun pointer'ını döndürür.
// type: İstenen operatör türü.
// Dönüş: İlgili operatör fonksiyonunun pointer'ı veya desteklenmiyorsa NULL.
c_ampersand_binary_operator_func c_ampersand_operator_get_binary_function(c_ampersand_operator_type type);

// ... operatör önceliği ve associativity ile ilgili tanımlamalar eklenebilir
// (Bunlar genellikle ayrıştırıcı (parser) veya semantik analizci tarafından kullanılır, runtime'da değil)

#endif // C_AMPERSAND_OPERATORS_H

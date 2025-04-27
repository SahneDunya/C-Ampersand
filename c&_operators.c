#include "c&_operators.h"
// C& veri tipleri (c_ampersand_type, c_ampersand_type_kind) için
#include "c&_types.h"
// C& değer yapıları ve value_equals gibi yardımcı fonksiyonlar için (c_ampersand_value, c_ampersand_value_equals)
#include "c&_data_types.h" // Varsayım: c_ampersand_value_equals burada bildirildi
// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT)
#include "c&_stdlib.h" // Varsayım: C_AMPERSAND_* hata kodları ve c_ampersand_type_bool burada tanımlı
#include <stdint.h> // Tam sayı tipleri (int32_t, float64_t vb.) için (c&_types.h de sağlayabilir)
#include <stdbool.h> // bool tipi için


// Helper function to check if a value is a numeric type.
// Relies on the assumption that TYPE_INT8 through TYPE_FLOAT64 are contiguous in c_ampersand_type_kind enum.
static c_ampersand_result check_numeric_type(const c_ampersand_value *val) {
    // NULL pointer kontrolü ekleyelim
    if (val == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Varsayım: TYPE_INT8, TYPE_INT16, ..., TYPE_FLOAT64 enum değerleri ardışık ve sıralı.
    if (val->type.kind >= TYPE_INT8 && val->type.kind <= TYPE_FLOAT64) {
        return C_AMPERSAND_OK_RESULT;
    }
    return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0}; // Tip uyuşmazlığı hatası
}

// --- İkili Operatör Implementasyonları ---

// İki sayısal değeri toplar.
// Şu an sadece aynı tipteki sayısal değerleri destekler.
static c_ampersand_result add_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sonuç value'yu sıfırla
    memset(result, 0, sizeof(c_ampersand_value));
    // result->type.kind = TYPE_UNKNOWN; // veya başka bir varsayılan

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) {
        return type_check1; // İlk işlenen sayısal değil
    }
     if (type_check2.code != C_AMPERSAND_OK) {
        return type_check2; // İkinci işlenen sayısal değil
    }

    // Şu anki basit implementasyon: Her iki operand da aynı türde olmalı.
    // Gerçek bir derleyicide burada tip dönüşümü (coercion) mantığı olurdu.
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0}; // Tip uyuşmazlığı hatası
    }

    // Sonuç tipi işlenenlerin tipiyle aynı olacak (basit örnek)
    result->type = operand1->type; // c_ampersand_type structı kopyalanır

    // Operasyonu tipine göre gerçekleştir
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val + operand2->data.i32_val;
            break;
        case TYPE_INT64: // long long
            result->data.i64_val = operand1->data.i64_val + operand2->data.i64_val;
            break;
        case TYPE_FLOAT32: // float
            result->data.f32_val = operand1->data.f32_val + operand2->data.f32_val;
            break;
        case TYPE_FLOAT64: // double
            result->data.float64_val = operand1->data.float64_val + operand2->data.float64_val;
            break;
        // ... diğer sayısal türler için eklemeler (UINT*, CHAR?)
        // String birleştirme (+) de buraya eklenebilir ancak sayısal değil.
        // String birleştirme genellikle ayrı bir operator fonksiyonu veya metot olarak ele alınır.
        default:
            // check_numeric_type zaten bu durumu ele almalı ama güvenlik için burada da kontrol edelim.
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0}; // Desteklenmeyen operasyon/tip
    }
    // Sonuç value'daki diğer alanları (örn. is_gc_managed) ayarlayın.
    // result->is_gc_managed = false; // Primitif tipler GC yönetimi gerektirmez

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// İki sayısal değeri birbirinden çıkarır. Aynı tip gereksinimi.
static c_ampersand_result subtract_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    memset(result, 0, sizeof(c_ampersand_value));

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) return type_check1;
    if (type_check2.code != C_AMPERSAND_OK) return type_check2;

    // Aynı tip gereksinimi
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    result->type = operand1->type;

    // Operasyonu tipine göre gerçekleştir
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val - operand2->data.i32_val;
            break;
        case TYPE_INT64:
            result->data.i64_val = operand1->data.i64_val - operand2->data.i64_val;
            break;
        case TYPE_FLOAT32:
            result->data.f32_val = operand1->data.f32_val - operand2->data.f32_val;
            break;
        case TYPE_FLOAT64:
            result->data.float64_val = operand1->data.float64_val - operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0};
    }
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT;
}

// İki sayısal değeri çarpar. Aynı tip gereksinimi.
static c_ampersand_result multiply_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
     // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    memset(result, 0, sizeof(c_ampersand_value));

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) return type_check1;
    if (type_check2.code != C_AMPERSAND_OK) return type_check2;

    // Aynı tip gereksinimi
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    result->type = operand1->type;

    // Operasyonu tipine göre gerçekleştir
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val * operand2->data.i32_val;
            break;
        case TYPE_INT64:
            result->data.i64_val = operand1->data.i64_val * operand2->data.i64_val;
            break;
        case TYPE_FLOAT32:
            result->data.f32_val = operand1->data.f32_val * operand2->data.f32_val;
            break;
        case TYPE_FLOAT64:
            result->data.float64_val = operand1->data.float64_val * operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0};
    }
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT;
}

// İki sayısal değeri böler. Aynı tip gereksinimi ve sıfıra bölme kontrolü.
static c_ampersand_result divide_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
     // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    memset(result, 0, sizeof(c_ampersand_value));

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) return type_check1;
    if (type_check2.code != C_AMPERSAND_OK) return type_check2;

    // Aynı tip gereksinimi
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    result->type = operand1->type;

    // Operasyonu tipine göre gerçekleştir ve sıfıra bölme kontrolü yap
    switch (operand1->type.kind) {
        case TYPE_INT32:
            if (operand2->data.i32_val == 0) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO, NULL, 0};
            result->data.i32_val = operand1->data.i32_val / operand2->data.i32_val;
            break;
        case TYPE_INT64:
            if (operand2->data.i64_val == 0) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO, NULL, 0};
            result->data.i64_val = operand1->data.i64_val / operand2->data.i64_val;
            break;
        case TYPE_FLOAT32:
            // Kayan nokta sıfıra bölme genellikle hata fırlatmaz (IEEE 754).
            // Ancak C& dilinin sıfıra bölme hatası fırlatması isteniyorsa bu kontrol gereklidir.
            if (operand2->data.f32_val == 0.0f) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO, NULL, 0};
            result->data.f32_val = operand1->data.f32_val / operand2->data.f32_val;
            break;
        case TYPE_FLOAT64:
            // Kayan nokta sıfıra bölme genellikle hata fırlatmaz (IEEE 754).
            // Ancak C& dilinin sıfıra bölme hatası fırlatması isteniyorsa bu kontrol gereklidir.
            if (operand2->data.float64_val == 0.0) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO, NULL, 0};
            result->data.float64_val = operand1->data.float64_val / operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0};
    }
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT;
}

// İki değeri eşitlik açısından karşılaştırır.
// c_ampersand_value_equals fonksiyonunu kullanır.
static c_ampersand_result equal_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
     // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sonuç value'yu sıfırla (bool değeri 0/false olur)
    memset(result, 0, sizeof(c_ampersand_value));

    // Varsayım: c_ampersand_type_bool c&_stdlib.h veya c&_types.h'de tanımlı
    result->type = c_ampersand_type_bool;
    // Varsayım: c_ampersand_value_equals c&_data_types.h'de bildirildi ve uygun tipler için implemente edildi.
    result->data.bool_val = c_ampersand_value_equals(operand1, operand2);

    // Sonuç value'nun GC durumu (bool değeri GC yönetimi gerektirmez)
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// İki değerin eşitsizliğini karşılaştırır. Eşitlik operatörünü kullanır.
static c_ampersand_result not_equal_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sonuç value'yu sıfırla
    memset(result, 0, sizeof(c_ampersand_value));

    // Eşitlik kontrolünü yap
    c_ampersand_result equal_res = equal_operator(operand1, operand2, result);
    if (equal_res.code != C_AMPERSAND_OK) {
        // Eşitlik kontrolünde hata olursa o hatayı döndür
        return equal_res;
    }

    // Eşitlik sonucunun tersini al
    result->data.bool_val = !result->data.bool_val;

    // Sonuç tipi ve GC durumu zaten equal_operator tarafından ayarlandı.

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// İki sayısal değeri küçüklük açısından karşılaştırır. Aynı tip gereksinimi.
static c_ampersand_result less_than_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
     // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sonuç value'yu sıfırla
    memset(result, 0, sizeof(c_ampersand_value));

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) return type_check1;
    if (type_check2.code != C_AMPERSAND_OK) return type_check2;

    // Aynı tip gereksinimi
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    // Sonuç tipi boolean
    result->type = c_ampersand_type_bool;

    // Operasyonu tipine göre gerçekleştir
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.bool_val = operand1->data.i32_val < operand2->data.i32_val;
            break;
        case TYPE_INT64:
            result->data.bool_val = operand1->data.i64_val < operand2->data.i64_val;
            break;
        case TYPE_FLOAT32:
            result->data.f32_val = operand1->data.f32_val < operand2->data.f32_val;
            break;
        case TYPE_FLOAT64:
            result->data.bool_val = operand1->data.float64_val < operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0};
    }
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT;
}

// İki sayısal değeri büyüklük açısından karşılaştırır. Aynı tip gereksinimi.
static c_ampersand_result greater_than_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
     // Parametre doğrulama
    if (operand1 == NULL || operand2 == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sonuç value'yu sıfırla
    memset(result, 0, sizeof(c_ampersand_value));

    // İşlenenlerin sayısal olup olmadığını kontrol et
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK) return type_check1;
    if (type_check2.code != C_AMPERSAND_OK) return type_check2;

    // Aynı tip gereksinimi
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    // Sonuç tipi boolean
    result->type = c_ampersand_type_bool;

    // Operasyonu tipine göre gerçekleştir
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.bool_val = operand1->data.i32_val > operand2->data.i32_val;
            break;
        case TYPE_INT64:
            result->data.bool_val = operand1->data.i64_val > operand2->data.i64_val;
            break;
        case TYPE_FLOAT32:
            result->data.f32_val = operand1->data.f32_val > operand2->data.f32_val;
            break;
        case TYPE_FLOAT64:
            result->data.bool_val = operand1->data.float64_val > operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, NULL, 0};
    }
    // result->is_gc_managed = false;

    return C_AMPERSAND_OK_RESULT;
}


// --- Operatör Fonksiyonlarını Getirme ---

// Belirli bir ikili operatör türü için implementasyon fonksiyonunun pointer'ını döndürür.
c_ampersand_binary_operator_func c_ampersand_operator_get_binary_function(c_ampersand_operator_type type) {
    switch (type) {
        case OPERATOR_ADD:      return add_operator;
        case OPERATOR_SUBTRACT: return subtract_operator;
        case OPERATOR_MULTIPLY: return multiply_operator;
        case OPERATOR_DIVIDE:   return divide_operator;
        case OPERATOR_EQUAL:    return equal_operator;
        case OPERATOR_NOT_EQUAL:return not_equal_operator;
        case OPERATOR_LESS_THAN:return less_than_operator;
        case OPERATOR_GREATER_THAN: return greater_than_operator;
        default:                return NULL; // Desteklenmeyen operatör tipi
    }
}

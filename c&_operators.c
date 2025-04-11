#include "c&_operators.h"
#include "c&_types.h"
#include "c&_data_types.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Gerekirse donanım hızlandırması veya sistem çağrıları için
#include <stdio.h>
#include <stdlib.h>

// --- Yardımcı Fonksiyonlar ---

// Tür uyumluluğunu kontrol eder ve hata döndürür (gerekirse).
static c_ampersand_result check_numeric_type(const c_ampersand_value *val) {
    if (val->type.kind >= TYPE_INT8 && val->type.kind <= TYPE_FLOAT64) {
        return C_AMPERSAND_OK_RESULT;
    }
    return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
}

// --- Operatör Implementasyonları ---

static c_ampersand_result add_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }

    // Basit bir örnek olarak, her iki operand da aynı türde olmalı
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }

    result->type = operand1->type; // Sonuç aynı türde olacak (basit örnek)
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val + operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            result->data.float64_val = operand1->data.float64_val + operand2->data.float64_val;
            break;
        // ... diğer sayısal türler için eklemeler
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result subtract_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Benzer mantıkla çıkarma işlemi
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    result->type = operand1->type;
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val - operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            result->data.float64_val = operand1->data.float64_val - operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result multiply_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Benzer mantıkla çarpma işlemi
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    result->type = operand1->type;
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.i32_val = operand1->data.i32_val * operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            result->data.float64_val = operand1->data.float64_val * operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result divide_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    // Benzer mantıkla bölme işlemi ve sıfıra bölme kontrolü
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    result->type = operand1->type;
    switch (operand1->type.kind) {
        case TYPE_INT32:
            if (operand2->data.i32_val == 0) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO};
            result->data.i32_val = operand1->data.i32_val / operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            if (operand2->data.float64_val == 0.0) return (c_ampersand_result){C_AMPERSAND_ERROR_DIVISION_BY_ZERO};
            result->data.float64_val = operand1->data.float64_val / operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result equal_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    result->type = c_ampersand_type_bool;
    result->data.bool_val = c_ampersand_value_equals(operand1, operand2);
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result not_equal_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    c_ampersand_result equal_res = equal_operator(operand1, operand2, result);
    if (equal_res.code != C_AMPERSAND_OK) return equal_res;
    result->data.bool_val = !result->data.bool_val;
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result less_than_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    result->type = c_ampersand_type_bool;
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.bool_val = operand1->data.i32_val < operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            result->data.bool_val = operand1->data.float64_val < operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result greater_than_operator(const c_ampersand_value *operand1, const c_ampersand_value *operand2, c_ampersand_value *result) {
    c_ampersand_result type_check1 = check_numeric_type(operand1);
    c_ampersand_result type_check2 = check_numeric_type(operand2);
    if (type_check1.code != C_AMPERSAND_OK || type_check2.code != C_AMPERSAND_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (operand1->type.kind != operand2->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    result->type = c_ampersand_type_bool;
    switch (operand1->type.kind) {
        case TYPE_INT32:
            result->data.bool_val = operand1->data.i32_val > operand2->data.i32_val;
            break;
        case TYPE_FLOAT64:
            result->data.bool_val = operand1->data.float64_val > operand2->data.float64_val;
            break;
        default:
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
    }
    return C_AMPERSAND_OK_RESULT;
}

// --- Operatör Fonksiyonlarını Getirme ---

c_ampersand_binary_operator_func c_ampersand_operator_get_binary_function(c_ampersand_operator_type type) {
    switch (type) {
        case OPERATOR_ADD:      return add_operator;
        case OPERATOR_SUBTRACT: return subtract_operator;
        case OPERATOR_MULTIPLY: return multiply_operator;
        case OPERATOR_DIVIDE:   return divide_operator;
        case OPERATOR_EQUAL:    return equal_operator;
        case OPERATOR_NOT_EQUAL:return not_equal_operator;
        case OPERATOR_LESS_THAN:return less_than_operator;
        case OPERATOR_GREATER_THAN: return greater_than_operator;
        default:                return NULL;
    }
}

// ... diğer operatör implementasyonları buraya eklenebilir
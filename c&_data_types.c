#include "c&_data_types.h"
#include "c&_types.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Gerekirse bellek yönetimi için
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// --- c&_types.h içinde tanımlanan harici değişkenlerin tanımı ---

const c_ampersand_type c_ampersand_type_void =    {TYPE_VOID};
const c_ampersand_type c_ampersand_type_bool =    {TYPE_BOOL};
const c_ampersand_type c_ampersand_type_int8 =    {TYPE_INT8};
const c_ampersand_type c_ampersand_type_int16 =   {TYPE_INT16};
const c_ampersand_type c_ampersand_type_int32 =   {TYPE_INT32};
const c_ampersand_type c_ampersand_type_int64 =   {TYPE_INT64};
const c_ampersand_type c_ampersand_type_uint8 =   {TYPE_UINT8};
const c_ampersand_type c_ampersand_type_uint16 =  {TYPE_UINT16};
const c_ampersand_type c_ampersand_type_uint32 =  {TYPE_UINT32};
const c_ampersand_type c_ampersand_type_uint64 =  {TYPE_UINT64};
const c_ampersand_type c_ampersand_type_float32 = {TYPE_FLOAT32};
const c_ampersand_type c_ampersand_type_float64 = {TYPE_FLOAT64};
const c_ampersand_type c_ampersand_type_string =  {TYPE_STRING};
const c_ampersand_type c_ampersand_type_pointer = {TYPE_POINTER};

const char *c_ampersand_type_to_string(c_ampersand_type type) {
    switch (type.kind) {
        case TYPE_VOID:    return "void";
        case TYPE_BOOL:    return "bool";
        case TYPE_INT8:    return "int8";
        case TYPE_INT16:   return "int16";
        case TYPE_INT32:   return "int32";
        case TYPE_INT64:   return "int64";
        case TYPE_UINT8:   return "uint8";
        case TYPE_UINT16:  return "uint16";
        case TYPE_UINT32:  return "uint32";
        case TYPE_UINT64:  return "uint64";
        case TYPE_FLOAT32: return "float32";
        case TYPE_FLOAT64: return "float64";
        case TYPE_STRING:  return "string";
        case TYPE_POINTER: return "pointer";
        default:           return "unknown";
    }
}

// --- c&_data_types.h içinde tanımlanan fonksiyonların implementasyonu ---

c_ampersand_result c_ampersand_value_create(c_ampersand_type type, c_ampersand_value *out_value) {
    if (out_value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    out_value->type = type;
    // Bazı tipler için başlangıç değerleri atanabilir
    if (type.kind == TYPE_STRING) {
        out_value->data.string_val = NULL; // Başlangıçta boş string
    } else if (type.kind == TYPE_POINTER) {
        out_value->data.pointer_val = NULL;
    }
    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_value_free(c_ampersand_value *value) {
    if (value == NULL) {
        return;
    }
    if (value->type.kind == TYPE_STRING && value->data.string_val != NULL) {
        free(value->data.string_val);
        value->data.string_val = NULL;
    }
    // Diğer serbest bırakılması gereken tipler için (örneğin, pointer'lar) buraya eklemeler yapılabilir.
}

c_ampersand_result c_ampersand_value_set_bool(c_ampersand_value *value, bool val) {
    if (value == NULL || value->type.kind != TYPE_BOOL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    value->data.bool_val = val;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_int(c_ampersand_value *value, int64_t val) {
    if (value == NULL || (value->type.kind < TYPE_INT8 || value->type.kind > TYPE_INT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Burada tür kontrolü ve dönüşüm yapılabilir (örneğin, int32_t'ye sığmayan bir değer verilirse hata).
    value->data.i64_val = val;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_uint(c_ampersand_value *value, uint64_t val) {
    if (value == NULL || (value->type.kind < TYPE_UINT8 || value->type.kind > TYPE_UINT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    value->data.u64_val = val;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_float(c_ampersand_value *value, double val) {
    if (value == NULL || (value->type.kind != TYPE_FLOAT32 && value->type.kind != TYPE_FLOAT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    if (value->type.kind == TYPE_FLOAT32) {
        value->data.float32_val = (float)val;
    } else {
        value->data.float64_val = val;
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_string(c_ampersand_value *value, const char *val) {
    if (value == NULL || value->type.kind != TYPE_STRING) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    if (value->data.string_val != NULL) {
        free(value->data.string_val);
    }
    value->data.string_val = strdup(val);
    if (value->data.string_val == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_pointer(c_ampersand_value *value, void *val) {
    if (value == NULL || value->type.kind != TYPE_POINTER) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    value->data.pointer_val = val;
    return C_AMPERSAND_OK_RESULT;
}

bool c_ampersand_value_get_bool(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_BOOL) {
        // Hata yönetimi eklenebilir
        return false;
    }
    return value->data.bool_val;
}

int64_t c_ampersand_value_get_int(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind < TYPE_INT8 || value->type.kind > TYPE_INT64)) {
        // Hata yönetimi eklenebilir
        return 0;
    }
    return value->data.i64_val;
}

uint64_t c_ampersand_value_get_uint(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind < TYPE_UINT8 || value->type.kind > TYPE_UINT64)) {
        // Hata yönetimi eklenebilir
        return 0;
    }
    return value->data.u64_val;
}

double c_ampersand_value_get_float(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind != TYPE_FLOAT32 && value->type.kind != TYPE_FLOAT64)) {
        // Hata yönetimi eklenebilir
        return 0.0;
    }
    if (value->type.kind == TYPE_FLOAT32) {
        return (double)value->data.float32_val;
    } else {
        return value->data.float64_val;
    }
}

const char *c_ampersand_value_get_string(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_STRING) {
        // Hata yönetimi eklenebilir
        return NULL;
    }
    return value->data.string_val;
}

void *c_ampersand_value_get_pointer(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_POINTER) {
        // Hata yönetimi eklenebilir
        return NULL;
    }
    return value->data.pointer_val;
}

bool c_ampersand_value_equals(const c_ampersand_value *val1, const c_ampersand_value *val2) {
    if (val1 == NULL || val2 == NULL || val1->type.kind != val2->type.kind) {
        return false;
    }
    switch (val1->type.kind) {
        case TYPE_BOOL:    return val1->data.bool_val == val2->data.bool_val;
        case TYPE_INT8:    return val1->data.i8_val == val2->data.i8_val;
        case TYPE_INT16:   return val1->data.i16_val == val2->data.i16_val;
        case TYPE_INT32:   return val1->data.i32_val == val2->data.i32_val;
        case TYPE_INT64:   return val1->data.i64_val == val2->data.i64_val;
        case TYPE_UINT8:   return val1->data.u8_val == val2->data.u8_val;
        case TYPE_UINT16:  return val1->data.u16_val == val2->data.u16_val;
        case TYPE_UINT32:  return val1->data.u32_val == val2->data.u32_val;
        case TYPE_UINT64:  return val1->data.u64_val == val2->data.u64_val;
        case TYPE_FLOAT32: return val1->data.float32_val == val2->data.float32_val;
        case TYPE_FLOAT64: return val1->data.float64_val == val2->data.float64_val;
        case TYPE_STRING:  return (val1->data.string_val == NULL && val2->data.string_val == NULL) ||
                                 (val1->data.string_val != NULL && val2->data.string_val != NULL &&
                                  strcmp(val1->data.string_val, val2->data.string_val) == 0);
        case TYPE_POINTER: return val1->data.pointer_val == val2->data.pointer_val;
        default:           return false;
    }
}
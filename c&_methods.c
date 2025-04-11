#include "c&_methods.h"
#include "c&_stdlib.h"
#include "c&_data_types.h"
#include "sahne64_api.h" // Gerekirse bellek yönetimi için
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Tüm metot tablolarının başı
c_ampersand_method_table *global_method_table = NULL;

// --- Metot Yönetimi ---

c_ampersand_method *c_ampersand_method_create(const char *name, c_ampersand_type_kind type, c_ampersand_method_func func_ptr) {
    c_ampersand_method *method = malloc(sizeof(c_ampersand_method));
    if (method == NULL) {
        return NULL;
    }
    method->name = strdup(name);
    if (method->name == NULL) {
        free(method);
        return NULL;
    }
    method->belongs_to_type = type;
    method->func_ptr = func_ptr;
    method->next = NULL;
    return method;
}

void c_ampersand_method_free(c_ampersand_method *method) {
    if (method == NULL) {
        return;
    }
    free(method->name);
    free(method);
}

c_ampersand_result c_ampersand_method_register(c_ampersand_type_kind type, c_ampersand_method *method) {
    if (method == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL && current_table->type != type) {
        current_table = current_table->next;
    }

    if (current_table == NULL) {
        // Bu tip için henüz bir metot tablosu yok, oluştur
        c_ampersand_method_table *new_table = malloc(sizeof(c_ampersand_method_table));
        if (new_table == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        new_table->type = type;
        new_table->methods = method;
        new_table->next = global_method_table;
        global_method_table = new_table;
    } else {
        // Mevcut tabloya metodu ekle
        method->next = current_table->methods;
        current_table->methods = method;
    }

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_method *c_ampersand_method_lookup(c_ampersand_type_kind object_type, const char *method_name) {
    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL && current_table->type != object_type) {
        current_table = current_table->next;
    }

    if (current_table != NULL) {
        c_ampersand_method *current_method = current_table->methods;
        while (current_method != NULL) {
            if (strcmp(current_method->name, method_name) == 0) {
                return current_method;
            }
            current_method = current_method->next;
        }
    }

    return NULL; // Metot bulunamadı
}

c_ampersand_result c_ampersand_method_call(c_ampersand_value *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    if (object == NULL || method_name == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_method *method = c_ampersand_method_lookup(object->type.kind, method_name);
    if (method == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, .message = "Metot bulunamadı"};
    }

    // Metodu çağır
    return method->func_ptr(object, args, num_args, result);
}

// --- Başlangıç ve Kapanış ---

c_ampersand_result c_ampersand_methods_init() {
    global_method_table = NULL;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_methods_shutdown() {
    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL) {
        c_ampersand_method_table *next_table = current_table->next;
        c_ampersand_method *current_method = current_table->methods;
        while (current_method != NULL) {
            c_ampersand_method *next_method = current_method->next;
            c_ampersand_method_free(current_method);
            current_method = next_method;
        }
        free(current_table);
        current_table = next_table;
    }
    global_method_table = NULL;
    return C_AMPERSAND_OK_RESULT;
}

// --- Örnek Metot Implementasyonları ---

c_ampersand_result string_length(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    if (self->type.kind != TYPE_STRING) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (num_args != 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, .message = "Bu metot argüman almaz."};
    }
    result->type = c_ampersand_type_int64;
    result->data.i64_val = strlen(self->data.string_val);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result int_to_string(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    if (self->type.kind != TYPE_INT32 && self->type.kind != TYPE_INT64) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }
    if (num_args != 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, .message = "Bu metot argüman almaz."};
    }
    char buffer[64];
    if (self->type.kind == TYPE_INT32) {
        snprintf(buffer, sizeof(buffer), "%d", self->data.i32_val);
    } else {
        snprintf(buffer, sizeof(buffer), "%lld", self->data.i64_val);
    }
    result->type = c_ampersand_type_string;
    result->data.string_val = strdup(buffer);
    if (result->data.string_val == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    return C_AMPERSAND_OK_RESULT;
}

// --- Örnek Metot Kayıt İşlemi (başka bir yerde yapılabilir) ---

c_ampersand_result register_example_methods() {
    c_ampersand_result result = C_AMPERSAND_OK_RESULT;
    c_ampersand_method *len_method = c_ampersand_method_create("length", TYPE_STRING, string_length);
    if (!len_method) return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    result = c_ampersand_method_register(TYPE_STRING, len_method);
    if (result.code != C_AMPERSAND_OK) return result;

    c_ampersand_method *to_str_method_int = c_ampersand_method_create("to_string", TYPE_INT32, int_to_string);
    if (!to_str_method_int) return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    result = c_ampersand_method_register(TYPE_INT32, to_str_method_int);
    if (result.code != C_AMPERSAND_OK) return result;

    c_ampersand_method *to_str_method_long = c_ampersand_method_create("to_string", TYPE_INT64, int_to_string);
    if (!to_str_method_long) return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    result = c_ampersand_method_register(TYPE_INT64, to_str_method_long);
    if (result.code != C_AMPERSAND_OK) return result;

    return C_AMPERSAND_OK_RESULT;
}
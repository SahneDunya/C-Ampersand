#include "c&_object.h"
#include "c&_stdlib.h"
#include "c&_types.h"
#include "c&_data_types.h"
#include "c&_methods.h"
#include "sahne64_api.h" // Bellek yönetimi için
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- Nesne Yönetimi ---

c_ampersand_result c_ampersand_object_create(c_ampersand_type type, c_ampersand_object **out_object) {
    if (out_object == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_object *object = malloc(sizeof(c_ampersand_object));
    if (object == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }

    object->type = type;
    object->data = NULL;

    // Veri için bellek ayır (type'a göre boyut belirlenmeli)
    size_t data_size = 0;
    switch (type.kind) {
        case TYPE_INT32:
            data_size = sizeof(int32_t);
            break;
        case TYPE_FLOAT64:
            data_size = sizeof(double);
            break;
        case TYPE_STRING:
            // String nesneleri genellikle bir pointer tutar, başlangıçta NULL olabilir
            data_size = sizeof(char *);
            break;
        // ... diğer tipler için boyutlar
        default:
            fprintf(stderr, "Uyarı: Bilinmeyen tip için nesne oluşturuluyor.\n");
            break;
    }

    if (data_size > 0) {
        object->data = sahne64_memory_allocate(data_size); // Sahne64 API'sı ile bellek ayırma
        if (object->data == NULL) {
            free(object);
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        // İsteğe bağlı: Veriyi sıfırla
        memset(object->data, 0, data_size);
    }

    *out_object = object;
    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_object_free(c_ampersand_object *object) {
    if (object == NULL) {
        return;
    }
    if (object->data != NULL) {
        // Eğer string ise, önce string verisini serbest bırak
        if (object->type.kind == TYPE_STRING && *(char **)object->data != NULL) {
            sahne64_memory_free(*(char **)object->data);
        }
        sahne64_memory_free(object->data); // Sahne64 API'sı ile bellek serbest bırakma
    }
    free(object);
}

c_ampersand_result c_ampersand_object_get_field(const c_ampersand_object *object, const char *field_name, c_ampersand_value *out_value) {
    if (object == NULL || field_name == NULL || out_value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    // Alan isimleri ve ofsetleri tip bilgisi içinde saklanmalıdır (basit örnekte atlanmıştır)
    // Gerçek bir implementasyonda, tip tanımından alan ofsetleri alınır.

    if (strcmp(field_name, "value") == 0) { // Örnek bir alan adı
        out_value->type = object->type;
        out_value->data = object->data; // Dikkat: Bu sadece basit tipler için geçerli. Derinlemesine kopyalama gerekebilir.
        if (object->type.kind == TYPE_STRING && object->data != NULL) {
            out_value->data.string_val = strdup(*(char **)object->data);
            if (out_value->data.string_val == NULL) {
                return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
            }
        }
        return C_AMPERSAND_OK_RESULT;
    }

    return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, .message = "Alan bulunamadı"};
}

c_ampersand_result c_ampersand_object_set_field(c_ampersand_object *object, const char *field_name, const c_ampersand_value *value) {
    if (object == NULL || field_name == NULL || value == NULL || object->type.kind != value->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    // Alan isimleri ve ofsetleri tip bilgisi içinde saklanmalıdır (basit örnekte atlanmıştır)
    // Gerçek bir implementasyonda, tip tanımından alan ofsetleri alınır.

    if (strcmp(field_name, "value") == 0) { // Örnek bir alan adı
        // Tür kontrolü zaten yapıldı
        if (object->type.kind == TYPE_STRING) {
            if (object->data != NULL && *(char **)object->data != NULL) {
                sahne64_memory_free(*(char **)object->data);
            }
            *(char **)object->data = strdup(value->data.string_val);
            if (*(char **)object->data == NULL) {
                return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
            }
        } else {
            memcpy(object->data, &value->data, c_ampersand_get_type_size(object->type));
        }
        return C_AMPERSAND_OK_RESULT;
    }

    return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, .message = "Alan bulunamadı"};
}

c_ampersand_result c_ampersand_object_call_method(c_ampersand_object *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    if (object == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Nesneyi (self) ilk argüman olarak metot çağrısına geçir
    return c_ampersand_method_call((c_ampersand_value *)object, method_name, args, num_args, result);
}

// --- Yardımcı Fonksiyon (c&_types.h içinde olabilir) ---
size_t c_ampersand_get_type_size(c_ampersand_type type) {
    switch (type.kind) {
        case TYPE_INT8: return sizeof(int8_t);
        case TYPE_INT16: return sizeof(int16_t);
        case TYPE_INT32: return sizeof(int32_t);
        case TYPE_INT64: return sizeof(int64_t);
        case TYPE_UINT8: return sizeof(uint8_t);
        case TYPE_UINT16: return sizeof(uint16_t);
        case TYPE_UINT32: return sizeof(uint32_t);
        case TYPE_UINT64: return sizeof(uint64_t);
        case TYPE_FLOAT32: return sizeof(float);
        case TYPE_FLOAT64: return sizeof(double);
        case TYPE_BOOL: return sizeof(bool);
        case TYPE_CHAR: return sizeof(char);
        case TYPE_STRING: return sizeof(char *);
        case TYPE_POINTER: return sizeof(void *);
        // ... diğer tipler
        default: return 0;
    }
}
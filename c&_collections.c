#include "c&_collections.h"
#include <stdlib.h>
#include <string.h>

c_ampersand_result c_ampersand_vector_create(size_t initial_capacity, size_t element_size, c_ampersand_vector *vector) {
    if (vector == NULL || element_size == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    vector->data = malloc(initial_capacity * element_size);
    if (vector->data == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->element_size = element_size;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_push(c_ampersand_vector *vector, const void *element) {
    if (vector == NULL || element == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    if (vector->size == vector->capacity) {
        size_t new_capacity = vector->capacity == 0 ? 4 : vector->capacity * 2; // Başlangıç kapasitesi veya iki katına çıkar
        void *new_data = realloc(vector->data, new_capacity * vector->element_size);
        if (new_data == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        vector->data = new_data;
        vector->capacity = new_capacity;
    }
    memcpy((char *)vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size++;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_pop(c_ampersand_vector *vector, void *element) {
    if (vector == NULL || vector->size == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Dizi boş
    }
    vector->size--;
    if (element != NULL) {
        memcpy(element, (char *)vector->data + vector->size * vector->element_size, vector->element_size);
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_get(const c_ampersand_vector *vector, size_t index, void *element) {
    if (vector == NULL || index >= vector->size || element == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz indeks veya null pointer
    }
    memcpy(element, (char *)vector->data + index * vector->element_size, vector->element_size);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_set(c_ampersand_vector *vector, size_t index, const void *element) {
    if (vector == NULL || index >= vector->size || element == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz indeks veya null pointer
    }
    memcpy((char *)vector->data + index * vector->element_size, element, vector->element_size);
    return C_AMPERSAND_OK_RESULT;
}

size_t c_ampersand_vector_size(const c_ampersand_vector *vector) {
    return vector == NULL ? 0 : vector->size;
}

size_t c_ampersand_vector_capacity(const c_ampersand_vector *vector) {
    return vector == NULL ? 0 : vector->capacity;
}

c_ampersand_result c_ampersand_vector_destroy(c_ampersand_vector *vector) {
    if (vector != NULL && vector->data != NULL) {
        free(vector->data);
        vector->data = NULL;
        vector->size = 0;
        vector->capacity = 0;
        vector->element_size = 0;
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer dinamik dizi fonksiyonlarının implementasyonları buraya gelebilir
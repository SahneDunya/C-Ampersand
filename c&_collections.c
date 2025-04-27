#include "c&_collections.h"
#include <string.h> // memcpy için hala lazım

// Varsayım: c&_stdlib.h içinde c_ampersand_result ve hata kodları tanımlı
/// Örneğin:
 typedef struct { int code; } c_ampersand_result;
 const c_ampersand_result C_AMPERSAND_OK_RESULT = { SAHNE_SUCCESS }; // SAHNE_SUCCESS = 0
 #define C_AMPERSAND_ERROR_OUT_OF_MEMORY     { 1 } // Veya SAHNE_ERROR_OUT_OF_MEMORY
 #define C_AMPERSAND_ERROR_INVALID_ARGUMENT  { 3 } // Veya SAHNE_ERROR_INVALID_PARAMETER
// NOT: Burada SAHNE hata kodlarını doğrudan C& hata kodlarına eşliyoruz gibi görünüyor.

c_ampersand_result c_ampersand_vector_create(size_t initial_capacity, size_t element_size, c_ampersand_vector *vector) {
    if (vector == NULL || element_size == 0) {
        // Varsayım: C_AMPERSAND_ERROR_INVALID_ARGUMENT c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    size_t allocation_size = initial_capacity * element_size;
    void *data_ptr = NULL;

    // Belleği SAHNE64 sistem çağrısı ile tahsis et
    // sahne_mem_allocate SAHNE_SUCCESS (0) dönerse data_ptr'a adresi yazar
    sahne_error_t sahne_err = sahne_mem_allocate(allocation_size, &data_ptr);

    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        // Varsayım: C_AMPERSAND_ERROR_OUT_OF_MEMORY c&_stdlib.h'de tanımlı
        // Ya da daha genel bir SAHNE hata eşleyicisi kullanılabilir
         return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
    }

    vector->data = data_ptr;
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->element_size = element_size;
    vector->allocated_bytes = allocation_size; // Ayrılan toplam byte boyutunu kaydet

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_push(c_ampersand_vector *vector, const void *element) {
    if (vector == NULL || element == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Vektörün SAHNE64'ten alınmış geçerli bir belleği olduğunu kontrol et
    if (vector->data == NULL && vector->capacity > 0) {
         // Tutarsız durum, create başarılı olmamış veya bozulmuş
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Veya yeni bir hata kodu
    }

    if (vector->size == vector->capacity) {
        size_t new_capacity = vector->capacity == 0 ? 4 : vector->capacity * 2;
        size_t new_allocation_size = new_capacity * vector->element_size;
        void *new_data_ptr = NULL;

        // Yeni belleği SAHNE64 sistem çağrısı ile tahsis et
        sahne_error_t sahne_err = sahne_mem_allocate(new_allocation_size, &new_data_ptr);

        if (sahne_err != SAHNE_SUCCESS) {
             return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
        }

        // Eski veriyi yeni konuma kopyala
        if (vector->data != NULL) {
             memcpy(new_data_ptr, vector->data, vector->size * vector->element_size);
        }

        // Eski belleği SAHNE64 sistem çağrısı ile serbest bırak
        // sahne_mem_release(NULL, size) güvenlidir, data NULL olabilir (initial_capacity 0 ise)
        sahne_err = sahne_mem_release(vector->data, vector->allocated_bytes);
        if (sahne_err != SAHNE_SUCCESS) {
            // Bellek serbest bırakma hatası ciddi olabilir, ama tahsis başarılıysa devam edebiliriz
            // Loglama veya farklı bir hata işleme stratejisi burada gerekebilir.
            // Şimdilik sadece OK dönüyoruz, ama dikkatli olmak lazım.
        }


        vector->data = new_data_ptr;
        vector->capacity = new_capacity;
        vector->allocated_bytes = new_allocation_size; // Yeni ayrılan boyutu kaydet
    }

    // Yeni elemanı kopyala
    memcpy((char *)vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size++;

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_pop(c_ampersand_vector *vector, void *element) {
    if (vector == NULL || vector->size == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Dizi boş veya null pointer
    }
    // Vektörün SAHNE64'ten alınmış geçerli bir belleği olduğunu kontrol et
    if (vector->data == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Bozulmuş vektör
    }


    vector->size--;
    if (element != NULL) {
        memcpy(element, (char *)vector->data + vector->size * vector->element_size, vector->element_size);
    }
    // Pop işleminde kapasite azaltma (shrink) yapılmıyor, bu daha karmaşık bir yeniden tahsis gerektirir.
    // Şimdilik sadece boyutu azaltmak yeterli.

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_get(const c_ampersand_vector *vector, size_t index, void *element) {
    if (vector == NULL || index >= vector->size || element == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz indeks veya null pointer
    }
    // Vektörün SAHNE64'ten alınmış geçerli bir belleği olduğunu kontrol et (redundant ama güvenli)
    if (vector->data == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Bozulmuş vektör
    }

    memcpy(element, (char *)vector->data + index * vector->element_size, vector->element_size);

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_vector_set(c_ampersand_vector *vector, size_t index, const void *element) {
    if (vector == NULL || index >= vector->size || element == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz indeks veya null pointer
    }
    // Vektörün SAHNE64'ten alınmış geçerli bir belleği olduğunu kontrol et (redundant ama güvenli)
    if (vector->data == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Bozulmuş vektör
    }

    memcpy((char *)vector->data + index * vector->element_size, element, vector->element_size);

    return C_AMPERSAND_OK_RESULT;
}

size_t c_ampersand_vector_size(const c_ampersand_vector *vector) {
    return (vector == NULL || vector->data == NULL) ? 0 : vector->size; // NULL vektör veya belleği olmayan vektör için 0
}

size_t c_ampersand_vector_capacity(const c_ampersand_vector *vector) {
    return (vector == NULL || vector->data == NULL) ? 0 : vector->capacity; // NULL vektör veya belleği olmayan vektör için 0
}

c_ampersand_result c_ampersand_vector_destroy(c_ampersand_vector *vector) {
    if (vector != NULL && vector->data != NULL) {
        // Belleği SAHNE64 sistem çağrısı ile serbest bırak
        sahne_error_t sahne_err = sahne_mem_release(vector->data, vector->allocated_bytes);

        // Bellek serbest bırakma hataları genellikle kurtarılamaz veya ciddi durumları işaret eder.
        // Dil seviyesinde bu hatayı nasıl ele alacağınız önemlidir (örn. panik, loglama).
        // C API'si olarak hata kodunu döndürüyoruz.
        if (sahne_err != SAHNE_SUCCESS) {
             // SAHNE hatasını C& hatasına çevirerek dön
             // SAHNE_ERROR_INVALID_ADDRESS veya SAHNE_ERROR_INVALID_PARAMETER olabilir
             return (c_ampersand_result){SAHNE_ERROR_to_C_AMPERSAND(sahne_err)}; // Varsayımsal bir eşleme makrosu
        }


        vector->data = NULL;
        vector->size = 0;
        vector->capacity = 0;
        vector->element_size = 0;
        vector->allocated_bytes = 0;
    }
    // NULL vektörü yok etmek bir hata değildir, sadece işlem yapmaz.
    return C_AMPERSAND_OK_RESULT;
}

// Varsayım: SAHNE_ERROR_to_C_AMPERSAND gibi bir makro veya fonksiyon c&_stdlib.h'de tanımlı
c_ampersand_result SAHNE_ERROR_to_C_AMPERSAND(sahne_error_t sahne_err);

// Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de const olarak tanımlı
const c_ampersand_result C_AMPERSAND_OK_RESULT = { SAHNE_SUCCESS };

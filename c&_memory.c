#include "c&_memory.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası

c_ampersand_result c_ampersand_allocate(size_t size, void **ptr) {
    if (ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    void *allocated_memory = sahne64_memory_allocate(size);
    if (allocated_memory == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    *ptr = allocated_memory;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_free(void *ptr) {
    if (ptr == NULL) {
        return C_AMPERSAND_OK_RESULT; // Null pointer'ı serbest bırakmak güvenlidir
    }
    int result = sahne64_memory_deallocate(ptr);
    if (result < 0) {
        // Sahne64 API'sında bellek serbest bırakma hatası olursa
        // Bu durum, çöp toplama ile entegrasyonun nasıl yapıldığına bağlı olabilir.
        // Belki de bu fonksiyon sadece belirli durumlarda (örneğin, manuel bellek yönetimi için) kullanılır.
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR}; // Genel bir IO hatası olarak değerlendirilebilir
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer bellek yönetimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
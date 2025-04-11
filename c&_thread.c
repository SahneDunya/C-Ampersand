#include "c&_thread.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası

c_ampersand_result c_ampersand_thread_create(c_ampersand_thread_entry_point entry_point, size_t stack_size, void *arg, c_ampersand_thread_id *thread_id) {
    sahne64_thread_id sahne_thread_id = sahne64_thread_spawn(entry_point, stack_size, arg);
    if (sahne_thread_id < 0) {
        switch (sahne_thread_id) {
            case -11: // EAGAIN (Kaynak geçici olarak kullanılamıyor) - Belki thread oluşturma hatası
                return (c_ampersand_result){C_AMPERSAND_ERROR_THREAD_CREATION_FAILED};
            case -12: // ENOMEM (Yeterli bellek yok)
                return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
            case -22: // EINVAL (Geçersiz argüman) - Belki geçersiz stack boyutu
                return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
            default:
                return (c_ampersand_result){C_AMPERSAND_ERROR_THREAD_CREATION_FAILED};
        }
    }
    *thread_id = sahne_thread_id;
    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_thread_exit(int code) {
    sahne64_thread_exit(code);
    // Bu noktaya asla ulaşılmaz, çünkü thread sonlanır.
}

// ... diğer thread yönetimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
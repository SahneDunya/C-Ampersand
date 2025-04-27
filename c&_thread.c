#include "c&_thread.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h" // c_ampersand_result, c_ampersand_error_with_message, map_sahne_error_to_camper_error için

// Sahne64 API başlık dosyası (sahne_thread_*, sahne_error_t için)
#include "sahne.h"

// c_ampersand_memory.h include edildi (OOM hatası mesajı için)
 #include "c&_memory.h" // map_sahne_error_to_camper_error veya error_with_message içinde kullanılıyor

#include <stddef.h> // size_t için
#include <stdint.h> // uint64_t için


// Yeni bir thread oluşturur (spawn).
c_ampersand_result c_ampersand_thread_create(c_ampersand_thread_entry_point entry_point, size_t stack_size, void *arg, c_ampersand_thread_id *thread_id) {
    // Parametre doğrulama
    if (entry_point == NULL || thread_id == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "thread_create: Geçersiz argüman (NULL entry_point veya thread_id pointer).");
    }
    // Başlangıçta thread ID'yi geçersiz yap
    *thread_id = C_AMPERSAND_INVALID_THREAD_ID;


    // Sahne64 API'sını kullanarak yeni bir thread başlat
    // Varsayım: sahne_thread_spawn sahne.h'de tanımlı, sahne_error_t döner ve ID'yi out_thread_id* parametresine yazar.
    u64 new_thread_id = C_AMPERSAND_INVALID_THREAD_ID;
    sahne_error_t sahne_err = sahne_thread_spawn(entry_point, stack_size, arg, &new_thread_id);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Thread oluşturma hatası. SAHNE hatasını C& hatasına çevirerek mesajlı dön.
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);

        // Belirli Sahne hatalarını daha spesifik C& hatalarına eşleyelim (orijinal koddaki gibi - varsayılan Sahne hataları için)
        // Varsayım: SAHNE_ERROR_OUT_OF_MEMORY, SAHNE_ERROR_INVALID_ARGUMENT, SAHNE_ERROR_RESOURCE_UNAVAILABLE (EAGAIN gibi) sahne.h'de tanımlı.
        switch(sahne_err) {
            case SAHNE_ERROR_OUT_OF_MEMORY:     camper_err_code = C_AMPERSAND_ERROR_OUT_OF_MEMORY; break;
            case SAHNE_ERROR_INVALID_ARGUMENT:  camper_err_code = C_AMPERSAND_ERROR_INVALID_ARGUMENT; break;
            case SAHNE_ERROR_RESOURCE_UNAVAILABLE: camper_err_code = C_AMPERSAND_ERROR_THREAD_CREATION_FAILED; break; // Kaynak geçici yok gibi
            default:                            camper_err_code = C_AMPERSAND_ERROR_THREAD_CREATION_FAILED; break; // Diğer hatalar
        }

        return c_ampersand_error_with_message(camper_err_code, "Thread oluşturulurken hata (SAHNE hatası: %d)", sahne_err);
    }

    // Başarılı olursa döndürülen thread ID'yi C& thread ID'sine ata
    *thread_id = new_thread_id;
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Mevcut (çağıran) thread'i sonlandırır. Bu fonksiyon geri dönmez.
void c_ampersand_thread_exit(int code) {
    // Sahne64 API'sını kullanarak thread'i sonlandır
    // Varsayım: sahne_thread_exit sahne.h'de tanımlı ve geri dönmez.
    sahne_thread_exit(code);
    // Bu noktaya asla ulaşılmaz, çünkü thread sonlanır.
}

// ... diğer thread yönetimi fonksiyonları (örn. get_thread_id, join)

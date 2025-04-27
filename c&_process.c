#include "c&_process.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_error_with_message için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (sahne_*, sahne_error_t, SAHNE_ERROR_*, u64 için)
#include "sahne.h"

#include <string.h> // strlen için
#include <stdint.h> // uint64_t için


// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
 static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);


// Yeni bir süreç oluşturur (spawn).
// path: Çalıştırılacak programın dosya yolu.
// pid: Oluşturulan sürecin ID'sinin yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_process_create(const char *path, c_ampersand_process_id *pid) {
    // Parametre doğrulama
    if (path == NULL || pid == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Başlangıçta PID'yi geçersiz yap
    *pid = C_AMPERSAND_INVALID_PROCESS_ID;


    // Sahne64 API'sını kullanarak yeni bir süreç başlat
    // Varsayım: sahne_process_spawn sahne.h'de tanımlı, sahne_error_t döner ve PID'yi out_pid* parametresine yazar.
    u64 new_pid = C_AMPERSAND_INVALID_PROCESS_ID;
    sahne_error_t sahne_err = sahne_process_spawn(path, strlen(path), &new_pid);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Süreç oluşturma hatası. SAHNE hatasını C& hatasına çevirerek mesajlı dön.
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);
        // Belirli Sahne hatalarını daha spesifik C& hatalarına eşleyelim (orijinal koddaki gibi)
        // Varsayım: SAHNE_ERROR_FILE_NOT_FOUND, SAHNE_ERROR_RESOURCE_UNAVAILABLE, SAHNE_ERROR_OUT_OF_MEMORY, SAHNE_ERROR_PERMISSION_DENIED sahne.h'de tanımlı.
        switch(sahne_err) {
            case SAHNE_ERROR_FILE_NOT_FOUND:    camper_err_code = C_AMPERSAND_ERROR_FILE_NOT_FOUND; break;
            case SAHNE_ERROR_RESOURCE_UNAVAILABLE: camper_err_code = C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED; break; // EAGAIN gibi
            case SAHNE_ERROR_OUT_OF_MEMORY:     camper_err_code = C_AMPERSAND_ERROR_OUT_OF_MEMORY; break;
            case SAHNE_ERROR_PERMISSION_DENIED: camper_err_code = C_AMPERSAND_ERROR_PERMISSION_DENIED; break;
            default:                            camper_err_code = C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED; break; // Diğer hatalar
        }

        return c_ampersand_error_with_message(camper_err_code, "Süreç oluşturulurken hata: %s (SAHNE hatası: %d)", path, sahne_err);
    }

    // Başarılı olursa döndürülen PID'yi C& PID'sine ata
    *pid = new_pid;
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Mevcut süreci sonlandırır. Bu fonksiyon geri dönmez.
// code: Sürecin çıkış kodu.
void c_ampersand_process_exit(int code) {
    // Sahne64 API'sını kullanarak süreci sonlandır
    // Varsayım: sahne_process_exit sahne.h'de tanımlı ve geri dönmez.
    sahne_process_exit(code);
    // Bu noktaya asla ulaşılmaz, çünkü süreç sonlanır.
}

// Mevcut (çağıran) sürecin ID'sini alır.
// pid: Mevcut sürecin ID'sinin yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_process_get_pid(c_ampersand_process_id *pid) {
    // Parametre doğrulama
    if (pid == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Başlangıçta PID'yi geçersiz yap
    *pid = C_AMPERSAND_INVALID_PROCESS_ID;

    // Sahne64 API'sını kullanarak mevcut sürecin ID'sini al
    // Varsayım: sahne_process_get_id sahne.h'de tanımlı, sahne_error_t döner ve PID'yi out_pid* parametresine yazar.
    sahne_error_t sahne_err = sahne_process_get_id(pid);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Süreç ID alma hatası. SAHNE hatasını C& hatasına çevirerek mesajlı dön.
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);
        return c_ampersand_error_with_message(camper_err_code, "Mevcut süreç ID'si alınırken hata (SAHNE hatası: %d)", sahne_err);
    }

    // Başarılı olursa PID *pid'ye zaten yazıldı.
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Süreci belirli bir süre uyutur (bloklar).
// milliseconds: Sürecin uyuyacağı milisaniye cinsinden süre.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_process_sleep(uint64_t milliseconds) {
    // Sahne64 API'sını kullanarak süreci uyut
    // Varsayım: sahne_sleep sahne.h'de tanımlı, sahne_error_t döner.
    sahne_error_t sahne_err = sahne_sleep(milliseconds);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Uyku hatası. SAHNE hatasını C& hatasına çevirerek mesajlı dön.
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);
        return c_ampersand_error_with_message(camper_err_code, "Süreç uyutulurken hata (SAHNE hatası: %d)", sahne_err);
    }

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// ... diğer süreç yönetimi fonksiyonları
// Örneğin:
 c_ampersand_result c_ampersand_process_wait(c_ampersand_process_id pid, int *out_exit_code);
 c_ampersand_result c_ampersand_process_kill(c_ampersand_process_id pid, int signal);

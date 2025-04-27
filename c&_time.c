#include "c&_time.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h" // c_ampersand_result, c_ampersand_error_with_message, map_sahne_error_to_camper_error için
// Süreç modülü (sleep fonksiyonu için)
#include "c&_process.h" // c_ampersand_process_sleep

// Sahne64 API başlık dosyası (sahne_time_*, sahne_error_t için)
#include "sahne.h"

#include <stdint.h> // uint64_t için


// Şu anki zamanı (timestamp olarak) alır.
c_ampersand_result c_ampersand_time_now(c_ampersand_timestamp *timestamp) {
    // Parametre doğrulama
    if (timestamp == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "time_now: Geçersiz argüman (timestamp pointer NULL).");
    }
    *timestamp = 0; // Başlangıçta sıfırla

    // Sahne64 API'sından şu anki zamanı al
    // Varsayım: sahne_time_get_now sahne.h'de tanımlı, sahne_error_t döner ve zamanı u64* parametresine yazar.
    sahne_error_t sahne_err = sahne_time_get_now(timestamp); // Timestamp direkt *timestamp'e yazılır

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Zaman alma hatası. SAHNE hatasını C& hatasına çevirerek mesajlı dön.
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);
        return c_ampersand_error_with_message(camper_err_code, "Şu anki zaman alınırken hata (SAHNE hatası: %d)", sahne_err);
    }

    // Başarı olursa timestamp zaten *timestamp'e yazıldı.
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Belirli bir süre bekler (milisaniye cinsinden).
// Bu fonksiyon, process modülündeki c_ampersand_process_sleep fonksiyonunu çağırır.
c_ampersand_result c_ampersand_time_sleep(uint64_t milliseconds) {
    // Bekleme işlevini process modülündeki sleep fonksiyonuna devret.
    // c_ampersand_process_sleep Sahne64 API'sını sarmalar ve hata yönetimi yapar.
    // Bu sayede bekleme mantığı tek bir yerde toplanır.
    return c_ampersand_process_sleep(milliseconds); // process_sleep'in sonucunu direkt ilet
}

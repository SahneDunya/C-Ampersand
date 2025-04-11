#include "c&_time.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası

c_ampersand_result c_ampersand_time_now(c_ampersand_timestamp *timestamp) {
    sahne64_timestamp_t current_time = sahne64_time_get_now();
    if (current_time < 0) {
        // Sahne64 API'sında zaman alma hatası olursa
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR}; // Genel bir IO hatası olarak değerlendirilebilir
    }
    *timestamp = (c_ampersand_timestamp)current_time;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_time_sleep(uint64_t milliseconds) {
    // Bu fonksiyon c&_process.c'deki c_ampersand_process_sleep fonksiyonunu çağırabilir
    // veya doğrudan Sahne64 API'sındaki bir uyku fonksiyonunu kullanabilir.
    // Burada doğrudan Sahne64 API'sını kullanmayı tercih edelim (eğer varsa).
    int result = sahne64_time_sleep(milliseconds);
    if (result < 0) {
        // Sahne64 API'sında uyku hatası olursa
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR}; // Genel bir IO hatası olarak değerlendirilebilir
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer zaman yönetimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
#include "c&_process.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası

c_ampersand_result c_ampersand_process_create(const char *path, c_ampersand_process_id *pid) {
    sahne64_pid_t sahne_pid = sahne64_process_spawn(path);
    if (sahne_pid < 0) {
        switch (sahne_pid) {
            case -3: // ESRCH (Böyle bir süreç yok) - Belki dosya bulunamadı anlamında kullanılabilir
                return (c_ampersand_result){C_AMPERSAND_ERROR_FILE_NOT_FOUND};
            case -11: // EAGAIN (Kaynak geçici olarak kullanılamıyor) - Belki süreç oluşturma hatası
                return (c_ampersand_result){C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED};
            case -12: // ENOMEM (Yeterli bellek yok)
                return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
            case -13: // EACCES (İzin reddedildi)
                return (c_ampersand_result){C_AMPERSAND_ERROR_PERMISSION_DENIED};
            default:
                return (c_ampersand_result){C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED};
        }
    }
    *pid = sahne_pid;
    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_process_exit(int code) {
    sahne64_process_exit(code);
    // Bu noktaya asla ulaşılmaz, çünkü süreç sonlanır.
}

c_ampersand_result c_ampersand_process_get_pid(c_ampersand_process_id *pid) {
    sahne64_pid_t current_pid = sahne64_process_id();
    if (current_pid < 0) {
        // Sahne64 API'sında süreç ID alma hatası olursa
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR}; // Genel bir IO hatası olarak değerlendirilebilir
    }
    *pid = current_pid;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_process_sleep(uint64_t milliseconds) {
    int result = sahne64_sleep(milliseconds);
    if (result < 0) {
        // Sahne64 API'sında uyku hatası olursa
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR}; // Genel bir IO hatası olarak değerlendirilebilir
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer süreç yönetimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
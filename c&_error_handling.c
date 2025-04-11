#include "c&_error_handling.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Hata mesajlarını almak için (gerekirse)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

c_ampersand_result c_ampersand_ok() {
    return (c_ampersand_result){C_AMPERSAND_OK, NULL};
}

c_ampersand_result c_ampersand_error(c_ampersand_error_code code) {
    return (c_ampersand_result){code, NULL};
}

c_ampersand_result c_ampersand_error_with_message(c_ampersand_error_code code, const char *message, ...) {
    char *formatted_message = NULL;
    if (message != NULL) {
        va_list args;
        va_start(args, message);
        int needed = vsnprintf(NULL, 0, message, args);
        va_end(args);

        formatted_message = malloc(needed + 1);
        if (formatted_message != NULL) {
            va_start(args, message);
            vsnprintf(formatted_message, needed + 1, message, args);
            va_end(args);
        }
    }
    return (c_ampersand_result){code, formatted_message};
}

bool c_ampersand_result_is_error(c_ampersand_result result) {
    return result.code != C_AMPERSAND_OK;
}

c_ampersand_error_code c_ampersand_result_get_code(c_ampersand_result result) {
    return result.code;
}

const char *c_ampersand_result_get_message(c_ampersand_result result) {
    return result.message;
}

void c_ampersand_result_print_error(c_ampersand_result result) {
    if (c_ampersand_result_is_error(result)) {
        fprintf(stderr, "Hata Kodu: %d", result.code);
        if (result.message != NULL) {
            fprintf(stderr, ", Mesaj: %s", result.message);
        }
        fprintf(stderr, "\n");
    }
}

// Örnek bir hata mesajı alma fonksiyonu (Sahne64 API'sından)
const char *sahne64_get_error_message(sahne64_result_t result_code) {
    // Bu sadece bir örnektir, gerçek implementasyon Sahne64 API'sına bağlıdır.
    switch (result_code) {
        case SAHNE64_ERROR_IO:
            return "Sahne64 I/O hatası oluştu.";
        case SAHNE64_ERROR_OUT_OF_MEMORY:
            return "Sahne64'te bellek yetersiz.";
        // ... diğer Sahne64 hata kodları için mesajlar
        default:
            return "Bilinmeyen Sahne64 hatası.";
    }
}
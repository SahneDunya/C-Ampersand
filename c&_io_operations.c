#include "c&_io_operations.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Sahne64 API fonksiyonları için
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Yardımcı Fonksiyonlar ---

// C Ampersand I/O modunu Sahne64 API moduna çevirir.
static sahne64_file_mode_t map_io_mode_to_sahne64(c_ampersand_io_mode mode) {
    switch (mode) {
        case C_AMPERSAND_IO_MODE_READ:
            return SAHNE64_FILE_MODE_READ;
        case C_AMPERSAND_IO_MODE_WRITE:
            return SAHNE64_FILE_MODE_WRITE;
        case C_AMPERSAND_IO_MODE_APPEND:
            return SAHNE64_FILE_MODE_APPEND;
        default:
            return SAHNE64_FILE_MODE_READ; // Varsayılan olarak okuma modu
    }
}

// --- I/O Operasyonlarının Implementasyonları ---

c_ampersand_result c_ampersand_io_open(const char *path, c_ampersand_io_mode mode, c_ampersand_file_handle *handle) {
    if (path == NULL || handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sahne64_file_mode_t sahne64_mode = map_io_mode_to_sahne64(mode);
    sahne64_result_t sahne64_result = sahne64_file_open(path, sahne64_mode, handle);
    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_io_close(c_ampersand_file_handle handle) {
    sahne64_result_t sahne64_result = sahne64_file_close(handle);
    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_io_read(c_ampersand_file_handle handle, void *buffer, size_t size, size_t *bytes_read) {
    if (buffer == NULL || bytes_read == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sahne64_result_t sahne64_result = sahne64_file_read(handle, buffer, size, bytes_read);
    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_io_write(c_ampersand_file_handle handle, const void *buffer, size_t size, size_t *bytes_written) {
    if (buffer == NULL || bytes_written == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sahne64_result_t sahne64_result = sahne64_file_write(handle, buffer, size, bytes_written);
    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_io_print(const char *message) {
    if (message == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sahne64_result_t sahne64_result = sahne64_console_print(message);
    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_io_read_line(c_ampersand_file_handle handle, char *buffer, size_t buffer_size, size_t *bytes_read) {
    if (buffer == NULL || bytes_read == NULL || buffer_size == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sahne64_result_t sahne64_result;
    if (handle == C_AMPERSAND_INVALID_FILE_HANDLE) {
        // Standart girdi
        sahne64_result = sahne64_console_read_line(buffer, buffer_size, bytes_read);
    } else {
        // Dosyadan okuma
        sahne64_result = sahne64_file_read_line(handle, buffer, buffer_size, bytes_read);
    }

    if (sahne64_result != SAHNE64_OK) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO, .message = sahne64_get_error_message(sahne64_result)};
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer I/O operasyonlarının implementasyonları buraya eklenebilir
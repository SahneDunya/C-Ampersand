#include "c&_io.h"
#include "sahne64_api.h" // Sahne64 API başlık dosyası

c_ampersand_result c_ampersand_file_open(const char *path, uint32_t flags, c_ampersand_file_descriptor *fd) {
    sahne64_fd_t sahne_fd = sahne64_open(path, flags);
    if (sahne_fd < 0) {
        switch (sahne_fd) {
            case -2: // ENOENT (Dosya veya dizin yok)
                return (c_ampersand_result){C_AMPERSAND_ERROR_FILE_NOT_FOUND};
            case -13: // EACCES (İzin reddedildi)
                return (c_ampersand_result){C_AMPERSAND_ERROR_PERMISSION_DENIED};
            default:
                return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR};
        }
    }
    *fd = sahne_fd;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_file_read(c_ampersand_file_descriptor fd, void *buffer, size_t count, size_t *bytes_read) {
    ssize_t result = sahne64_read(fd, buffer, count);
    if (result < 0) {
        if (result == -9) { // EBADF (Kötü dosya numarası)
            return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz dosya tanımlayıcısı olarak değerlendirilebilir
        }
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR};
    }
    if (bytes_read != NULL) {
        *bytes_read = (size_t)result;
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_file_write(c_ampersand_file_descriptor fd, const void *buffer, size_t count, size_t *bytes_written) {
    ssize_t result = sahne64_write(fd, buffer, count);
    if (result < 0) {
        if (result == -9) { // EBADF (Kötü dosya numarası)
            return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz dosya tanımlayıcısı olarak değerlendirilebilir
        }
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR};
    }
    if (bytes_written != NULL) {
        *bytes_written = (size_t)result;
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_file_close(c_ampersand_file_descriptor fd) {
    int result = sahne64_close(fd);
    if (result < 0) {
        if (result == -9) { // EBADF (Kötü dosya numarası)
            return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz dosya tanımlayıcısı olarak değerlendirilebilir
        }
        return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR};
    }
    return C_AMPERSAND_OK_RESULT;
}

// Standart çıktı için varsayılan dosya tanımlayıcısı (genellikle 1)
#define C_AMPERSAND_STDOUT_FD 1

c_ampersand_result c_ampersand_print(const char *str) {
    size_t len = c_ampersand_string_length(str);
    size_t bytes_written;
    return c_ampersand_file_write(C_AMPERSAND_STDOUT_FD, str, len, &bytes_written);
}

c_ampersand_result c_ampersand_println(const char *str) {
    c_ampersand_result result = c_ampersand_print(str);
    if (result.code == C_AMPERSAND_OK) {
        result = c_ampersand_print("\n");
    }
    return result;
}

// Basit bir string uzunluğu fonksiyonu (c&_string.c'de daha gelişmiş bir sürümü olabilir)
size_t c_ampersand_string_length(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
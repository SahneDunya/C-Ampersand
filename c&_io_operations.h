#ifndef C_AMPERSAND_IO_OPERATIONS_H
#define C_AMPERSAND_IO_OPERATIONS_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_types.h"       // Veri tipleri için
#include "c&_data_types.h" // Değerleri tutmak için

// Dosya erişim modları
typedef enum {
    C_AMPERSAND_IO_MODE_READ,
    C_AMPERSAND_IO_MODE_WRITE,
    C_AMPERSAND_IO_MODE_APPEND,
    // ... diğer modlar eklenebilir
} c_ampersand_io_mode;

// Dosya tanıtıcısı (Sahne64 API'sından gelebilir)
typedef sahne64_file_handle_t c_ampersand_file_handle;

// Geçersiz dosya tanıtıcısı
#define C_AMPERSAND_INVALID_FILE_HANDLE SAHNE64_INVALID_FILE_HANDLE

// Bir dosyayı açar.
c_ampersand_result c_ampersand_io_open(const char *path, c_ampersand_io_mode mode, c_ampersand_file_handle *handle);

// Bir dosyayı kapatır.
c_ampersand_result c_ampersand_io_close(c_ampersand_file_handle handle);

// Bir dosyadan veri okur.
c_ampersand_result c_ampersand_io_read(c_ampersand_file_handle handle, void *buffer, size_t size, size_t *bytes_read);

// Bir dosyaya veri yazar.
c_ampersand_result c_ampersand_io_write(c_ampersand_file_handle handle, const void *buffer, size_t size, size_t *bytes_written);

// Standart çıktıya bir string yazar.
c_ampersand_result c_ampersand_io_print(const char *message);

// Standart girdi veya bir dosyadan bir satır okur.
c_ampersand_result c_ampersand_io_read_line(c_ampersand_file_handle handle, char *buffer, size_t buffer_size, size_t *bytes_read);

// ... diğer I/O operasyonları (örneğin, dosya konumunu değiştirme) eklenebilir

#endif // C_AMPERSAND_IO_OPERATIONS_H
#ifndef C_AMPERSAND_IO_H
#define C_AMPERSAND_IO_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için

// Dosya açma modları (c&_stdlib.h'de de tanımlanmıştır, tekrar burada belirtilebilir)
#define C_AMPERSAND_O_RDONLY SAHNE64_O_RDONLY
#define C_AMPERSAND_O_WRONLY SAHNE64_O_WRONLY
#define C_AMPERSAND_O_RDWR   SAHNE64_O_RDWR
#define C_AMPERSAND_O_CREAT  SAHNE64_O_CREAT
#define C_AMPERSAND_O_EXCL   SAHNE64_O_EXCL
#define C_AMPERSAND_O_TRUNC  SAHNE64_O_TRUNC

// Dosya tanımlayıcısı
typedef sahne64_fd_t c_ampersand_file_descriptor;

// Fonksiyon prototipleri
c_ampersand_result c_ampersand_file_open(const char *path, uint32_t flags, c_ampersand_file_descriptor *fd);
c_ampersand_result c_ampersand_file_read(c_ampersand_file_descriptor fd, void *buffer, size_t count, size_t *bytes_read);
c_ampersand_result c_ampersand_file_write(c_ampersand_file_descriptor fd, const void *buffer, size_t count, size_t *bytes_written);
c_ampersand_result c_ampersand_file_close(c_ampersand_file_descriptor fd);
c_ampersand_result c_ampersand_print(const char *str);
c_ampersand_result c_ampersand_println(const char *str);

#endif // C_AMPERSAND_IO_H
#ifndef C_AMPERSAND_PACKAGE_FORMAT_H
#define C_AMPERSAND_PACKAGE_FORMAT_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include "c&_io.h"     // Dosya I/O işlemleri için
#include <stddef.h> // size_t için
#include <stdbool.h>

// C Ampersand paketinin temel meta bilgilerini tutan yapı
typedef struct {
    char *name;
    char *version;
    char *description;
    // ... diğer meta bilgiler (yazarlar, bağımlılıklar vb.) eklenebilir
} c_ampersand_package_metadata;

// Bir C Ampersand paket dosyasını açar.
c_ampersand_result c_ampersand_package_open(const char *file_path, c_ampersand_file_descriptor *package_fd);

// Bir C Ampersand paket dosyasını kapatır.
c_ampersand_result c_ampersand_package_close(c_ampersand_file_descriptor package_fd);

// Açık bir C Ampersand paketinden meta bilgileri okur.
c_ampersand_result c_ampersand_package_read_metadata(c_ampersand_file_descriptor package_fd, c_ampersand_package_metadata *metadata);

// Açık bir C Ampersand paketinden belirli bir dosyayı çıkarır.
c_ampersand_result c_ampersand_package_extract_file(c_ampersand_file_descriptor package_fd, const char *file_name, const char *destination_path);

// Açık bir C Ampersand paketindeki tüm dosyaların listesini alır.
c_ampersand_result c_ampersand_package_list_files(c_ampersand_file_descriptor package_fd);

// Bir dizinden C Ampersand paketi oluşturur.
c_ampersand_result c_ampersand_package_create(const char *source_directory, const char *output_file_path);

// Bir C Ampersand paket dosyasının geçerliliğini kontrol eder.
c_ampersand_result c_ampersand_package_validate(const char *file_path);

// ... diğer paket formatı ile ilgili fonksiyonlar eklenebilir (örneğin, imza kontrolü)

#endif // C_AMPERSAND_PACKAGE_FORMAT_H
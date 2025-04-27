#ifndef C_AMPERSAND_IO_H
#define C_AMPERSAND_IO_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle tipi, resource fonksiyonları ve MODE_* bayrakları için)
#include "sahne.h" // Varsayım: Sahne64 API'si u64 tipini tanımlar

#include <stddef.h> // size_t için
#include <stdint.h> // uint32_t için

// Dosya açma modları (Sahne64 MODE_* bayraklarına eşlenir)
// Varsayım: Bu değerler sahne.h içindeki MODE_* sabitlerine eşittir.
#define C_AMPERSAND_O_RDONLY MODE_READ
#define C_AMPERSAND_O_WRONLY MODE_WRITE
#define C_AMPERSAND_O_RDWR   (MODE_READ | MODE_WRITE)
#define C_AMPERSAND_O_CREAT  MODE_CREATE
#define C_AMPERSAND_O_EXCL   MODE_EXCLUSIVE
#define C_AMPERSAND_O_TRUNC  MODE_TRUNCATE
// Diğer Sahne64 MODE_* bayrakları buraya eklenebilir.

// C& dosya/kaynak tanımlayıcısı (Sahne64 Handle'ına karşılık gelir)
// SAHNE64 API'sında Handle genellikle u64 olarak temsil edilir.
typedef u64 c_ampersand_file_descriptor; // Sahne64 Handle'ı için u64 kullanıyoruz

// Standart çıktı kaynağı için Handle (GC tarafından yönetilen bir objeye pointer OLMAMALI!)
// Bu handle genellikle C& runtime'ı başlatılırken dışarıdan atanır.
extern u64 c_ampersand_stdout_handle; // SAHNE64 stdout Handle'ı


// Fonksiyon prototipleri
// Kaynak/dosya açar (Sahne64 resource_acquire'a karşılık gelir).
c_ampersand_result c_ampersand_file_open(const char *path, uint32_t flags, c_ampersand_file_descriptor *fd);

// Kaynaktan okur (Sahne64 resource_read'e karşılık gelir).
c_ampersand_result c_ampersand_file_read(c_ampersand_file_descriptor fd, void *buffer, size_t count, size_t *bytes_read);

// Kaynağa yazar (Sahne64 resource_write'a karşılık gelir).
c_ampersand_result c_ampersand_file_write(c_ampersand_file_descriptor fd, const void *buffer, size_t count, size_t *bytes_written);

// Kaynağı kapatır/serbest bırakır (Sahne64 resource_release'e karşılık gelir).
c_ampersand_result c_ampersand_file_close(c_ampersand_file_descriptor fd);

// Standart çıktıya bir string yazdırır.
c_ampersand_result c_ampersand_print(const char *str);

// Standart çıktıya bir string yazdırır ve satır sonu ekler.
c_ampersand_result c_ampersand_println(const char *str);

// Temel bir string uzunluğu hesaplama fonksiyonu
size_t c_ampersand_string_length(const char *str);


#endif // C_AMPERSAND_IO_H

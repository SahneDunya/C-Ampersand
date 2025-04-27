#ifndef C_AMPERSAND_IO_OPERATIONS_H
#define C_AMPERSAND_IO_OPERATIONS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle tipi, resource fonksiyonları ve MODE_* bayrakları için)
#include "sahne.h" // Varsayım: Sahne64 API'si u64, Handle, sahne_error_t, MODE_* tanımlarını içerir.

#include <stddef.h> // size_t için
#include <stdint.h> // uint32_t için

// Dosya/kaynak açma bayrakları (Sahne64 MODE_* bayraklarına eşlenir)
// Varsayım: Bu değerler sahne.h içindeki MODE_* sabitlerine eşittir.
#define C_AMPERSAND_O_RDONLY MODE_READ     // Okuma modu
#define C_AMPERSAND_O_WRONLY MODE_WRITE    // Yazma modu
#define C_AMPERSAND_O_RDWR   (MODE_READ | MODE_WRITE) // Okuma ve Yazma modu
#define C_AMPERSAND_O_CREAT  MODE_CREATE   // Yoksa oluştur
#define C_AMPERSAND_O_EXCL   MODE_EXCLUSIVE // Oluşturulacaksa, zaten varsa hata ver
#define C_AMPERSAND_O_TRUNC  MODE_TRUNCATE // Varsa içeriği sil
#define C_AMPERSAND_O_APPEND MODE_APPEND   // Yazmaları sona ekle
// Diğer Sahne64 MODE_* bayrakları buraya eklenebilir.

// C& dosya/kaynak tanıtıcısı (Sahne64 Handle'ına karşılık gelir)
// SAHNE64 API'sında Handle genellikle u64 olarak temsil edilir.
typedef u64 c_ampersand_file_handle; // Sahne64 Handle'ı için u64 kullanıyoruz

// Geçersiz dosya tanıtıcısı değeri (Sahne64 geçersiz Handle değerine eşittir)
#define C_AMPERSAND_INVALID_FILE_HANDLE 0 // Varsayım: SAHNE64'te geçersiz Handle 0

// Standart G/Ç Handle'ları (runtime tarafından atanması gereken global değişkenler)
// Bu Handle'lar C& runtime'ı başlatılırken dışarıdan alınır ve atanır.
extern u64 c_ampersand_stdin_handle;  // Standart girdi Handle'ı
extern u64 c_ampersand_stdout_handle; // Standart çıktı Handle'ı
extern u64 c_ampersand_stderr_handle; // Standart hata çıktı Handle'ı


// Fonksiyon prototipleri
// Kaynak/dosya açar (Sahne64 resource_acquire'a karşılık gelir). Path Sahne64 Resource ID formatında olmalı.
c_ampersand_result c_ampersand_io_open(const char *path, uint32_t flags, c_ampersand_file_handle *handle);

// Kaynağı kapatır/serbest bırakır (Sahne64 resource_release'e karşılık gelir).
c_ampersand_result c_ampersand_io_close(c_ampersand_file_handle handle);

// Kaynaktan veri okur (Sahne64 resource_read'e karşılık gelir).
c_ampersand_result c_ampersand_io_read(c_ampersand_file_handle handle, void *buffer, size_t size, size_t *bytes_read);

// Kaynağa veri yazar (Sahne64 resource_write'a karşılık gelir).
c_ampersand_result c_ampersand_io_write(c_ampersand_file_handle handle, const void *buffer, size_t size, size_t *bytes_written);

// Standart çıktıya bir string yazar (stdout Handle'ını kullanır).
c_ampersand_result c_ampersand_io_print(const char *message);

// Standart çıktıya bir string yazar ve satır sonu ekler (stdout Handle'ını kullanır).
c_ampersand_result c_ampersand_io_println(const char *message);

// Bir kaynaktan (stdin veya dosya) bir satır okur.
// Varsayım: Sahne64 API'si resource_read_line veya console_read_line sağlar, veya burada manuel implementasyon yapılır.
// Basitlik adına resource_read_line benzeri bir API çağrısı varsayılacaktır.
c_ampersand_result c_ampersand_io_read_line(c_ampersand_file_handle handle, char *buffer, size_t buffer_size, size_t *bytes_read);

// Temel bir string uzunluğu hesaplama fonksiyonu
size_t c_ampersand_string_length(const char *str);


// ... diğer I/O operasyonları (örneğin, dosya konumunu değiştirme - seek) eklenebilir

#endif // C_AMPERSAND_IO_OPERATIONS_H

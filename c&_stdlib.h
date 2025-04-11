#ifndef C_AMPERSAND_STDLIB_H
#define C_AMPERSAND_STDLIB_H

// C Ampersand Standart Kütüphanesi
// Rust ve C temelli, Sahne64 API'sına bağımlı

#include <stddef.h> // size_t gibi temel tipler için
#include <stdint.h> // uint64_t, int32_t gibi kesin boyutlu tipler için
#include <stdbool.h> // bool, true, false için (Rust'taki gibi)

// Sahne64 API Başlık Dosyası (varsayalım ki bu isimde bir dosyanız var)
#include "sahne64_api.h"

// --- Temel Veri Tipleri (c&_types.h'de de tanımlanmış olabilir) ---
typedef uint8_t c_ampersand_u8;
typedef uint16_t c_ampersand_u16;
typedef uint32_t c_ampersand_u32;
typedef uint64_t c_ampersand_u64;

typedef int8_t c_ampersand_i8;
typedef int16_t c_ampersand_i16;
typedef int32_t c_ampersand_i32;
typedef int64_t c_ampersand_i64;

typedef float c_ampersand_f32;
typedef double c_ampersand_f64;

// --- Hata Yönetimi (Rust'taki Result benzeri bir yapı) ---
typedef enum {
    C_AMPERSAND_OK = 0,
    C_AMPERSAND_ERROR_OUT_OF_MEMORY,
    C_AMPERSAND_ERROR_INVALID_ARGUMENT,
    C_AMPERSAND_ERROR_FILE_NOT_FOUND,
    C_AMPERSAND_ERROR_PERMISSION_DENIED,
    C_AMPERSAND_ERROR_IO_ERROR,
    C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED,
    C_AMPERSAND_ERROR_THREAD_CREATION_FAILED,
    C_AMPERSAND_ERROR_MUTEX_ERROR,
    C_AMPERSAND_ERROR_IPC_ERROR,
    // ... diğer olası hatalar
    C_AMPERSAND_ERROR_UNKNOWN
} c_ampersand_result_code;

typedef struct {
    c_ampersand_result_code code;
    // İstenirse ek hata bilgisi eklenebilir
} c_ampersand_result;

// Başarılı bir sonucu kolayca oluşturmak için makro
#define C_AMPERSAND_OK_RESULT { C_AMPERSAND_OK }

// --- Giriş/Çıkış İşlemleri (Sahne64 fs modülünü kullanır) ---

// Dosya açma modları (Sahne64'teki ile uyumlu)
#define C_AMPERSAND_O_RDONLY SAHNE64_O_RDONLY
#define C_AMPERSAND_O_WRONLY SAHNE64_O_WRONLY
#define C_AMPERSAND_O_RDWR   SAHNE64_O_RDWR
#define C_AMPERSAND_O_CREAT  SAHNE64_O_CREAT
#define C_AMPERSAND_O_EXCL   SAHNE64_O_EXCL
#define C_AMPERSAND_O_TRUNC  SAHNE64_O_TRUNC

// Dosya tanımlayıcısı (Sahne64'teki ile aynı olabilir)
typedef sahne64_fd_t c_ampersand_file_descriptor;

// Dosya açma
c_ampersand_result c_ampersand_file_open(const char *path, uint32_t flags, c_ampersand_file_descriptor *fd);

// Dosyadan okuma
c_ampersand_result c_ampersand_file_read(c_ampersand_file_descriptor fd, void *buffer, size_t count, size_t *bytes_read);

// Dosyaya yazma
c_ampersand_result c_ampersand_file_write(c_ampersand_file_descriptor fd, const void *buffer, size_t count, size_t *bytes_written);

// Dosyayı kapatma
c_ampersand_result c_ampersand_file_close(c_ampersand_file_descriptor fd);

// Standart çıktıya yazdırma (Sahne64 API'sını kullanabilir)
c_ampersand_result c_ampersand_print(const char *str);
c_ampersand_result c_ampersand_println(const char *str);

// --- Süreç Yönetimi (Sahne64 process modülünü kullanır) ---

// Süreç ID'si
typedef sahne64_pid_t c_ampersand_process_id;

// Yeni bir süreç oluşturma
c_ampersand_result c_ampersand_process_create(const char *path, c_ampersand_process_id *pid);

// Mevcut süreci sonlandırma
void c_ampersand_process_exit(int code);

// Mevcut sürecin ID'sini alma
c_ampersand_result c_ampersand_process_get_pid(c_ampersand_process_id *pid);

// Süreci uyutma
c_ampersand_result c_ampersand_process_sleep(uint64_t milliseconds);

// --- Thread Yönetimi (Sahne64 process modülünü kullanır) ---

// Thread ID'si
typedef sahne64_thread_id_t c_ampersand_thread_id;

// Yeni bir thread oluşturma (entry_point bir fonksiyon pointer'ı olmalı)
typedef void (*c_ampersand_thread_entry_point)(void *);
c_ampersand_result c_ampersand_thread_create(c_ampersand_thread_entry_point entry_point, size_t stack_size, void *arg, c_ampersand_thread_id *thread_id);

// Mevcut thread'i sonlandırma
void c_ampersand_thread_exit(int code);

// --- Senkronizasyon (Sahne64 sync modülünü kullanır) ---

// Muteks ID'si
typedef sahne64_mutex_id_t c_ampersand_mutex_id;

// Muteks oluşturma
c_ampersand_result c_ampersand_mutex_create(c_ampersand_mutex_id *mutex_id);

// Muteksi kilitleme
c_ampersand_result c_ampersand_mutex_lock(c_ampersand_mutex_id mutex_id);

// Muteksi kilidini açma
c_ampersand_result c_ampersand_mutex_unlock(c_ampersand_mutex_id mutex_id);

// --- Bellek Yönetimi (Otomatik bellek yönetimi dilin bir parçası olsa da temel fonksiyonlar olabilir) ---

// Belirli boyutta bellek ayırma (çöp toplayıcı ile etkileşimli olabilir)
c_ampersand_result c_ampersand_allocate(size_t size, void **ptr);

// Belleği serbest bırakma (çöp toplayıcı tarafından otomatik yapılsa da gerekebilir)
c_ampersand_result c_ampersand_free(void *ptr);

// --- String İşlemleri (Basit örnekler) ---
size_t c_ampersand_string_length(const char *str);
// ... diğer string fonksiyonları eklenebilir

// --- Matematiksel Fonksiyonlar (Basit örnekler) ---
 int c_ampersand_abs(int n);
// ... diğer matematiksel fonksiyonlar eklenebilir

// --- Diğer Yardımcı Fonksiyonlar ---

#endif // C_AMPERSAND_STDLIB_H
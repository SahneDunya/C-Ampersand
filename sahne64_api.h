#ifndef SAHNE64_API_H
#define SAHNE64_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// --- Temel Tipler ve Sonuç Yapısı ---

// Sahne64 API'sinden dönen genel sonuç tipi
typedef enum {
    SAHNE64_OK = 0,
    SAHNE64_ERROR_GENERIC,
    SAHNE64_ERROR_OUT_OF_MEMORY,
    SAHNE64_ERROR_INVALID_ARGUMENT,
    SAHNE64_ERROR_IO,
    SAHNE64_ERROR_NOT_FOUND,
    SAHNE64_ERROR_UNSUPPORTED_OPERATION,
    // ... diğer Sahne64 hata kodları
} sahne64_result_t;

// Hata mesajlarını almak için (örnek fonksiyon)
const char *sahne64_get_error_message(sahne64_result_t result_code);

// --- Bellek Yönetimi ---

// Bellek ayırır.
void *sahne64_memory_allocate(size_t size);

// Ayrılan belleği serbest bırakır.
void sahne64_memory_free(void *ptr);

// --- Dosya İşlemleri ---

// Dosya tanıtıcısı
typedef intptr_t sahne64_file_handle_t;
#define SAHNE64_INVALID_FILE_HANDLE -1

// Dosya açma modları
typedef enum {
    SAHNE64_FILE_MODE_READ,
    SAHNE64_FILE_MODE_WRITE,
    SAHNE64_FILE_MODE_APPEND,
    // ... diğer modlar
} sahne64_file_mode_t;

// Bir dosyayı açar.
sahne64_result_t sahne64_file_open(const char *path, sahne64_file_mode_t mode, sahne64_file_handle_t *handle);

// Bir dosyayı kapatır.
sahne64_result_t sahne64_file_close(sahne64_file_handle_t handle);

// Bir dosyadan veri okur.
sahne64_result_t sahne64_file_read(sahne64_file_handle_t handle, void *buffer, size_t size, size_t *bytes_read);

// Bir dosyaya veri yazar.
sahne64_result_t sahne64_file_write(sahne64_file_handle_t handle, const void *buffer, size_t size, size_t *bytes_written);

// Bir dosyadan bir satır okur.
sahne64_result_t sahne64_file_read_line(sahne64_file_handle_t handle, char *buffer, size_t buffer_size, size_t *bytes_read);

// --- Konsol İşlemleri ---

// Konsola bir string yazar.
sahne64_result_t sahne64_console_print(const char *message);

// Konsoldan bir satır okur.
sahne64_result_t sahne64_console_read_line(char *buffer, size_t buffer_size, size_t *bytes_read);

// --- Değişken Yönetimi (Örnek - eğer API seviyesinde yönetiliyorsa) ---
typedef intptr_t sahne64_variable_handle_t;
sahne64_result_t sahne64_variable_create(const char *name, sahne64_type_t type, sahne64_variable_handle_t *handle);
sahne64_result_t sahne64_variable_set_value(sahne64_variable_handle_t handle, const void *value);
sahne64_result_t sahne64_variable_get_value(sahne64_variable_handle_t handle, void *value);
sahne64_result_t sahne64_variable_destroy(sahne64_variable_handle_t handle);

// --- Tip Sistemi (Eğer API seviyesinde tanımlanıyorsa) ---
typedef enum {
    SAHNE64_TYPE_UNKNOWN,
    SAHNE64_TYPE_INT8,
    SAHNE64_TYPE_INT16,
    SAHNE64_TYPE_INT32,
    SAHNE64_TYPE_INT64,
    SAHNE64_TYPE_UINT8,
    SAHNE64_TYPE_UINT16,
    SAHNE64_TYPE_UINT32,
    SAHNE64_TYPE_UINT64,
    SAHNE64_TYPE_FLOAT32,
    SAHNE64_TYPE_FLOAT64,
    SAHNE64_TYPE_BOOL,
    SAHNE64_TYPE_CHAR,
    SAHNE64_TYPE_STRING,
    SAHNE64_TYPE_POINTER,
    // ... diğer tipler
} sahne64_type_t;

// --- Sabit Değerler (Eğer API seviyesinde tanımlanıyorsa) ---
typedef struct {
    sahne64_type_t type;
    union {
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        bool boolean;
        char character;
        const char *string;
        void *pointer;
    } value;
} sahne64_constant_t;

// --- Register Yönetimi (Eğer API seviyesinde yapılıyorsa) ---
typedef intptr_t sahne64_register_t;
#define SAHNE64_INVALID_REGISTER -1

// ... register ile ilgili fonksiyonlar (örneğin, al, serbest bırak, yükle)

// --- Label Yönetimi (Eğer kod üretimi API seviyesindeyse) ---
typedef intptr_t sahne64_label_t;
#define SAHNE64_INVALID_LABEL -1

// ... label ile ilgili fonksiyonlar (örneğin, oluştur, tanımla, atla)

// --- Diğer Sahne64 API Fonksiyonları (Gerektikçe eklenebilir) ---

#endif // SAHNE64_API_H
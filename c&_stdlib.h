#ifndef C_AMPERSAND_STDLIB_H
#define C_AMPERSAND_STDLIB_H

// C Ampersand Standart Kütüphanesi Temelleri
// Rust ve C temelli, Sahne64 API'sına bağımlı

// Temel C tipleri (Sahne64 araç zinciri tarafından sağlanmalı)
#include <stddef.h> // size_t gibi temel tipler için
#include <stdint.h> // uint64_t, int32_t gibi kesin boyutlu tipler için
#include <stdbool.h> // bool, true, false için

// Sahne64 API Başlık Dosyası (Temel Sahne tipleri ve hata kodları için)
#include "sahne.h" // sahne_error_t, SAHNE_SUCCESS, u64, vsnprintf vb. için varsayım


// --- Temel C& Veri Tipleri ---
// Sahne64'ün sağladığı temel tiplerin alias'ları veya C&'e özel temel tipler.
// c&_types.h daha karmaşık/dil seviyesi tipleri (struct, array vb.) tanımlar.
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
    C_AMPERSAND_OK = 0, // Başarılı durum

    // Genel Hatalar
    C_AMPERSAND_ERROR_UNKNOWN = 1,
    C_AMPERSAND_ERROR_OUT_OF_MEMORY,
    C_AMPERSAND_ERROR_INVALID_ARGUMENT,
    C_AMPERSAND_ERROR_NULL_POINTER, // NULL pointer hatası

    // Sistem Kaynakları Hataları
    C_AMPERSAND_ERROR_IO_ERROR, // Genel Giriş/Çıkış Hatası
    C_AMPERSAND_ERROR_FILE_NOT_FOUND,
    C_AMPERSAND_ERROR_PERMISSION_DENIED,
    C_AMPERSAND_ERROR_RESOURCE_UNAVAILABLE, // Kaynak (örn. dosya, socket) geçici olarak kullanılamıyor
    C_AMPERSAND_ERROR_LIMIT_REACHED, // Sistem limiti aşıldı (örn. max dosya, max süreç)

    // Süreç/Thread Hataları
    C_AMPERSAND_ERROR_PROCESS_CREATION_FAILED,
    C_AMPERSAND_ERROR_THREAD_CREATION_FAILED,
    C_AMPERSAND_ERROR_PROCESS_NOT_FOUND, // Belirtilen PID'de süreç yok
    C_AMPERSAND_ERROR_WAIT_FAILED, // Süreç bekleme hatası
    C_AMPERSAND_ERROR_SIGNAL_FAILED, // Sinyal gönderme hatası

    // Senkronizasyon Hataları
    C_AMPERSAND_ERROR_MUTEX_ERROR, // Genel mutex hatası
    C_AMPERSAND_ERROR_DEADLOCK, // Kilitlenme tespit edildi
    C_AMPERSAND_ERROR_ALREADY_LOCKED, // Zaten kilitli kaynağı kilitleme girişimi

    // Ağ Hataları (Varsayımsal)
    C_AMPERSAND_ERROR_NETWORK_ERROR, // Genel Ağ Hatası
    C_AMPERSAND_ERROR_CONNECTION_REFUSED,
    C_AMPERSAND_ERROR_CONNECTION_RESET,
    C_AMPERSAND_ERROR_HOST_NOT_FOUND,

    // Dil/Runtime Hataları
    C_AMPERSAND_ERROR_TYPE_MISMATCH,
    C_AMPERSAND_ERROR_DIVISION_BY_ZERO,
    C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION,
    C_AMPERSAND_ERROR_SEMANTIC, // Semantik analiz hatası
    C_AMPERSAND_ERROR_RUNTIME, // Çalışma zamanı hatası
    C_AMPERSAND_ERROR_INDEX_OUT_OF_BOUNDS, // Dizi/buffer dışı erişim
    C_AMPERSAND_ERROR_NULL_REFERENCE, // Null referans hatası
    C_AMPERSAND_ERROR_PACKAGE_FORMAT, // Paket formatı geçersiz
    C_AMPERSAND_ERROR_PACKAGE_ALREADY_INSTALLED, // Paket zaten kurulu
    C_AMPERSAND_ERROR_PACKAGE_NOT_INSTALLED, // Paket kurulu değil
    C_AMPERSAND_ERROR_REPOSITORY_NOT_FOUND, // Depo bulunamadı
    // ... Diğer modül/dil spesifik hatalar eklenebilir

    // Özel Hata Kodları İçin Başlangıç (modüller kendi özel hatalarını tanımlayabilir)
    C_AMPERSAND_ERROR_CUSTOM_START = 100,

    // Hata kodu sayısı (veya son geçerli kod)
    C_AMPERSAND_ERROR_COUNT

} c_ampersand_result_code;

// Fonksiyon sonuçlarını ve hata bilgilerini (mesaj dahil) tutan yapı
typedef struct {
    c_ampersand_result_code code; // Sonuç kodu
    char *message; // Hata mesajı (heap'te tahsis edilmiş olabilir)
    size_t message_allocated_size; // Mesaj stringi için ayrılan boyut (free için)
} c_ampersand_result;

// Başarılı bir sonucu kolayca oluşturmak için makro (mesaj alanlarını sıfırlar)
#define C_AMPERSAND_OK_RESULT { C_AMPERSAND_OK, NULL, 0 }

// Mesaj içeren bir hata c_ampersand_result yapısı oluşturur.
// Message stringi heap'te tahsis edilir (c&_stdlib.c içinde implemente edilir).
c_ampersand_result c_ampersand_error_with_message(c_ampersand_result_code code, const char *format, ...);

// Bir c_ampersand_result yapısındaki hata mesajı stringini serbest bırakır.
// result yapısının kendisini serbest BIRAKMAZ.
// (c&_stdlib.c içinde implemente edilir).
c_ampersand_result c_ampersand_result_free_message(c_ampersand_result *result);

// Sahne64 hata kodunu C& hata koduna eşler (c&_stdlib.c içinde implemente edilir).
c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);


// --- Modül Başlıklarının Dahil Edilmesi ---
// Standart kütüphanenin parçası olan diğer modüllerin arayüzlerini sağlar.
#include "c&_types.h" // Veri tipleri tanımı
#include "c&_memory.h" // Bellek yönetimi (allocate, free)
#include "c&_io_operations.h" // Giriş/Çıkış (dosya, konsol)
#include "c&_process.h" // Süreç yönetimi
#include "c&_string.h" // String yardımcı fonksiyonları

// --- Konsol Giriş/Çıkış (Formatlı) ---
// Standart çıktıya yazdırma (c&_stdlib.c içinde implemente edilir).
// vsnprintf ve Sahne64 konsol API'sını kullanır.
c_ampersand_result c_ampersand_print(const char *format, ...);
c_ampersand_result c_ampersand_println(const char *format, ...);


// --- Matematiksel Fonksiyonlar ---
// Basit örnek, daha fazlası c&_math.h/c'de olabilir.
int c_ampersand_abs(int n); // c&_stdlib.c içinde implemente edilir.


// --- Diğer Yardımcı Fonksiyonlar ---
// Zaman, tarih, rastgele sayılar, çevre değişkenleri vb. için fonksiyonlar eklenebilir.
// Bunlar da ayrı modüller (c&_time.h, c&_random.h, c&_env.h) ve .c dosyaları gerektirir.


#endif // C_AMPERSAND_STDLIB_H

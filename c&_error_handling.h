#ifndef C_AMPERSAND_ERROR_HANDLING_H
#define C_AMPERSAND_ERROR_HANDLING_H

// c_ampersand_result yapısı ve C_AMPERSAND_* hata kodu sabitleri için
// Varsayım: c_ampersand_result structı { c_ampersand_error_code code; char *message; size_t message_size; } şeklindedir.
#include "c&_stdlib.h"

// bool tipi için (bazı toolchain'lerde stdlib.h veya stddef.h değil, stdbool.h gerekir)
#include <stdbool.h>

// C Ampersand hata kodları
// Varsayım: Bu enum c&_stdlib.h içinde veya burada tanımlı. Burada tanımlayalım.
typedef enum {
    C_AMPERSAND_OK = 0,
    C_AMPERSAND_ERROR_GENERIC,
    C_AMPERSAND_ERROR_OUT_OF_MEMORY,         // Bellek yetersizliği (SAHNE64 OOM ile eşleşebilir)
    C_AMPERSAND_ERROR_INVALID_ARGUMENT,      // Geçersiz fonksiyon argümanı (SAHNE64 InvalidParameter ile eşleşebilir)
    C_AMPERSAND_ERROR_INVALID_SYNTAX,        // Ayrıştırma/sözdizimi hatası
    C_AMPERSAND_ERROR_SEMANTIC,          // Anlamsal hata (örn. tanımsız değişken)
    C_AMPERSAND_ERROR_TYPE_MISMATCH,     // Tip uyuşmazlığı
    C_AMPERSAND_ERROR_DIVISION_BY_ZERO,  // Sıfıra bölme
    C_AMPERSAND_ERROR_IO,                // G/Ç hatası (SAHNE64 kaynak hataları ile eşleşebilir)
    C_AMPERSAND_ERROR_CODEGEN_FAILED,    // Kod üretme hatası
    C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, // Desteklenmeyen işlem (SAHNE64 NotSupported ile eşleşebilir)
    C_AMPERSAND_ERROR_DEPENDENCY_CYCLE,  // Bağımlılık döngüsü (dependency_analyzer'dan)
    C_AMPERSAND_ERROR_DEPENDENCY_NOT_MET, // Bağımlılık karşılanmadı (dependency_resolver'dan)
    C_AMPERSAND_ERROR_LIMIT_EXCEEDED,    // Sistem/kaynak limiti aşıldı (dependency_analyzer'dan MAX_PACKAGES gibi)
    C_AMPERSAND_ERROR_REPOSITORY_ERROR,  // Paket deposu hatası (repository'den)
    // ... diğer hata kodları eklenebilir
    C_AMPERSAND_ERROR_COUNT // Enum boyutu için
} c_ampersand_error_code;


// Başarılı bir sonuç döndürür.
// Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de const olarak tanımlı ve bu fonksiyon onu döndürür.
c_ampersand_result c_ampersand_ok();

// Belirli bir hata koduyla bir hata sonucu döndürür (mesajsız).
c_ampersand_result c_ampersand_error(c_ampersand_error_code code);

// Belirli bir hata kodu ve formatlanmış mesajıyla bir hata sonucu döndürür.
// Mesaj stringi SAHNE64 belleğinde ayrılır.
c_ampersand_result c_ampersand_error_with_message(c_ampersand_error_code code, const char *message_format, ...);

// Bir sonucun hata içerip içermediğini kontrol eder.
bool c_ampersand_result_is_error(c_ampersand_result result);

// Bir sonuçtan hata kodunu alır.
c_ampersand_error_code c_ampersand_result_get_code(c_ampersand_result result);

// Bir sonuçtan hata mesajını alır (varsa).
const char *c_ampersand_result_get_message(c_ampersand_result result);

// Bir hata sonucunu C& çıktı mekanizmasını kullanarak yazdırır.
// SAHNE64 kaynak G/Ç'sini kullanır (c_ampersand_println aracılığıyla).
void c_ampersand_result_print_error(c_ampersand_result result);

// Bir c_ampersand_result yapısı içindeki hata mesajı belleğini serbest bırakır (varsa).
// SAHNE64 bellek serbest bırakmayı kullanır.
void c_ampersand_result_free(c_ampersand_result *result);


// Örnek: Sahne64 hata kodunu C& hata koduna çevirme fonksiyonu (isteğe bağlı)
// Varsayım: Sahne64 hata kodları sahne.h'de tanımlı
 c_ampersand_result c_ampersand_error_from_sahne_error(sahne_error_t sahne_err);

#endif // C_AMPERSAND_ERROR_HANDLING_H

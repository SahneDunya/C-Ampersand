#ifndef C_AMPERSAND_ERROR_HANDLING_H
#define C_AMPERSAND_ERROR_HANDLING_H

#include "c&_stdlib.h" // c_ampersand_result yapısı için

// C Ampersand hata kodları
typedef enum {
    C_AMPERSAND_OK = 0,
    C_AMPERSAND_ERROR_GENERIC,
    C_AMPERSAND_ERROR_OUT_OF_MEMORY,
    C_AMPERSAND_ERROR_INVALID_ARGUMENT,
    C_AMPERSAND_ERROR_INVALID_SYNTAX,
    C_AMPERSAND_ERROR_SEMANTIC,
    C_AMPERSAND_ERROR_TYPE_MISMATCH,
    C_AMPERSAND_ERROR_DIVISION_BY_ZERO,
    C_AMPERSAND_ERROR_IO,
    C_AMPERSAND_ERROR_CODEGEN_FAILED,
    C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION,
    // ... diğer hata kodları eklenebilir
    C_AMPERSAND_ERROR_COUNT
} c_ampersand_error_code;

// Başarılı bir sonuç döndürür.
c_ampersand_result c_ampersand_ok();

// Belirli bir hata koduyla bir hata sonucu döndürür.
c_ampersand_result c_ampersand_error(c_ampersand_error_code code);

// Belirli bir hata kodu ve mesajıyla bir hata sonucu döndürür.
c_ampersand_result c_ampersand_error_with_message(c_ampersand_error_code code, const char *message, ...);

// Bir sonucun hata içerip içermediğini kontrol eder.
bool c_ampersand_result_is_error(c_ampersand_result result);

// Bir sonuçtan hata kodunu alır.
c_ampersand_error_code c_ampersand_result_get_code(c_ampersand_result result);

// Bir sonuçtan hata mesajını alır (varsa).
const char *c_ampersand_result_get_message(c_ampersand_result result);

// Bir hata sonucunu standart hata akışına yazdırır.
void c_ampersand_result_print_error(c_ampersand_result result);

#endif // C_AMPERSAND_ERROR_HANDLING_H
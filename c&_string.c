#include "c&_string.h"
#include <string.h> // Standart C string fonksiyonları için
#include <stdio.h>  // sprintf için (string'den int'e dönüşümde kullanılabilir)
#include <stdlib.h> // atoi için (string'den int'e dönüşümde kullanılabilir)
#include <stdbool.h>

size_t c_ampersand_string_length(const char *str) {
    return strlen(str);
}

c_ampersand_result c_ampersand_string_copy(char *dest, const char *src) {
    if (dest == NULL || src == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    strcpy(dest, src);
    return C_AMPERSAND_OK_RESULT;
}

int c_ampersand_string_compare(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

c_ampersand_result c_ampersand_string_concatenate(char *dest, const char *s1, const char *s2) {
    if (dest == NULL || s1 == NULL || s2 == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    strcpy(dest, s1);
    strcat(dest, s2);
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_string_substring(char *dest, const char *src, size_t start, size_t length) {
    if (dest == NULL || src == NULL || start + length > strlen(src)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    strncpy(dest, src + start, length);
    dest[length] = '\0'; // Null terminatör eklemeyi unutmayın
    return C_AMPERSAND_OK_RESULT;
}

bool c_ampersand_string_contains_char(const char *str, char c) {
    return strchr(str, c) != NULL;
}

bool c_ampersand_string_contains_string(const char *str, const char *substring) {
    return strstr(str, substring) != NULL;
}

bool c_ampersand_string_starts_with(const char *str, char c) {
    return str != NULL && *str == c;
}

bool c_ampersand_string_starts_with_string(const char *str, const char *prefix) {
    return str != NULL && prefix != NULL && strncmp(str, prefix, strlen(prefix)) == 0;
}

bool c_ampersand_string_ends_with(const char *str, char c) {
    if (str == NULL) return false;
    size_t len = strlen(str);
    return len > 0 && str[len - 1] == c;
}

bool c_ampersand_string_ends_with_string(const char *str, const char *suffix) {
    if (str == NULL || suffix == NULL) return false;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return str_len >= suffix_len && strcmp(str + (str_len - suffix_len), suffix) == 0;
}

int c_ampersand_string_index_of_char(const char *str, char c) {
    char *ptr = strchr(str, c);
    if (ptr == NULL) {
        return -1;
    }
    return (int)(ptr - str);
}

int c_ampersand_string_index_of_string(const char *str, const char *substring) {
    char *ptr = strstr(str, substring);
    if (ptr == NULL) {
        return -1;
    }
    return (int)(ptr - str);
}

c_ampersand_result c_ampersand_string_from_int(char *dest, int value) {
    if (dest == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    sprintf(dest, "%d", value);
    return C_AMPERSAND_OK_RESULT;
}

int c_ampersand_string_to_int(const char *str) {
    if (str == NULL) {
        return 0; // Hata durumunda 0 döndürülebilir, daha gelişmiş hata yönetimi düşünülebilir
    }
    return atoi(str);
}

// ... diğer string fonksiyonlarının implementasyonları buraya gelebilir
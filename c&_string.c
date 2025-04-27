#include "c&_string.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h"
// c_ampersand_allocate/free için
#include "c&_memory.h" // String fonksiyonları bellek tahsisi yapabilir.

// Standart C string fonksiyonları için (Sahne toolchain sağlıyor varsayımıyla)
#include <string.h>
// vsnprintf, snprintf için (Sahne toolchain sağlıyor varsayımıyla)
#include <stdio.h> // snprintf için
// Sayısal dönüşümler için (Sahne toolchain sağlıyorsa kullanılır, yoksa implemente edilir)
#include <stdlib.h> // atoi için

#include <stdbool.h>


// String uzunluğunu döndürür (null sonlandırıcı hariç).
// str NULL ise tanımsız davranış veya hata döndürülebilir.
size_t c_ampersand_string_length(const char *str) {
    if (str == NULL) {
        // NULL string için uzunluk 0 kabul edilebilir veya özel bir değer döndürülebilir.
        // Standart C strlen NULL için tanımsızdır. 0 dönmek en güvenlisi.
        return 0;
    }
    return strlen(str);
}

// Kaynak string'i hedef buffer'a kopyalar.
c_ampersand_result c_ampersand_string_copy(char *dest, const char *src) {
    // Parametre doğrulama
    if (dest == NULL || src == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_copy: Hedef veya kaynak string NULL.");
    }
    // NOT: Hedef buffer'ın yeterince büyük olduğundan emin olma sorumluluğu çağırandadır.
    // strcpy kullanmak güvenlidir çünkü src null sonlandırıcıya sahiptir.
    strcpy(dest, src);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// İki string'i karşılaştırır.
int c_ampersand_string_compare(const char *s1, const char *s2) {
    // Parametre doğrulama - strcmp NULL için tanımsızdır.
    if (s1 == NULL || s2 == NULL) {
        // NULL stringler için özel karşılaştırma mantığı (örn. NULL her zaman küçüktür veya hata).
        // Şimdilik NULL'u boş string gibi davranalım veya özel bir değer dönelim.
         return (s1 == s2) ? 0 : (s1 == NULL ? -1 : 1); // NULL boş stringden küçüktür varsayımı
        // Daha güvenlisi: Hata olduğunu belirten bir değer dönmek. Ama int dönmesi bekleniyor.
         c_ampersand_println("STRING UYARI: string_compare NULL argüman aldı.");
         // Tanımsız davranışa neden olmamak için strcmp'ye geçmeyelim.
         // 0 dönmek eşit gibi davranır ama yanlıştır. Non-zero dönmek daha iyi.
         return (s1 == s2) ? 0 : (s1 == NULL ? -1 : 1); // Kaba hata dönüşü
    }
    return strcmp(s1, s2);
}

// İki string'i birleştirir ve sonucu hedef buffer'a yazar.
c_ampersand_result c_ampersand_string_concatenate(char *dest, const char *s1, const char *s2) {
    // Parametre doğrulama
    if (dest == NULL || s1 == NULL || s2 == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_concatenate: Geçersiz argüman (NULL pointer).");
    }
     // NOT: Hedef buffer'ın yeterli boyutta olduğundan emin olma sorumluluğu çağırandadır (strlen(s1) + strlen(s2) + 1).

    // strcpy ve strcat kullanmak güvenlidir çünkü s1 ve s2 null sonlandırıcılara sahiptir.
    strcpy(dest, s1); // s1'i kopyala
    strcat(dest, s2); // s2'yi s1'in sonuna ekle (dest null sonlandırıcıyı bulur)

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir string'in alt string'ini oluşturur ve hedef buffer'a kopyalar.
c_ampersand_result c_ampersand_string_substring(char *dest, const char *src, size_t start, size_t length) {
    // Parametre doğrulama
    if (dest == NULL || src == NULL) {
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_substring: Geçersiz argüman (NULL pointer).");
    }

    size_t src_len = strlen(src);
    // start ve length'in src stringi içinde geçerli bir aralığı belirttiğini kontrol et
    if (start > src_len || start + length > src_len) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_substring: Geçersiz alt string aralığı (start=%zu, length=%zu, src_len=%zu).", start, length, src_len);
    }
     // NOT: Hedef buffer'ın en az length + 1 boyutta olduğundan emin olma sorumluluğu çağırandadır.

    // strncpy kullan (alt stringi kopyalamak için)
    strncpy(dest, src + start, length);
    dest[length] = '\0'; // strncpy null sonlandırıcısını her zaman eklemez, manuel ekle.

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir string'in belirli bir karakteri içerip içermediğini kontrol eder.
bool c_ampersand_string_contains_char(const char *str, char c) {
    if (str == NULL) return false; // NULL string karakter içermez
    return strchr(str, c) != NULL; // strchr NULL bulamazsa NULL döner
}

// Bir string'in belirli bir alt string'i içerip içermediğini kontrol eder.
bool c_ampersand_string_contains_string(const char *str, const char *substring) {
     if (str == NULL || substring == NULL) return false; // NULL string/substring içermez
     if (strlen(substring) == 0) return true; // Boş string her stringde bulunur
    return strstr(str, substring) != NULL; // strstr NULL bulamazsa NULL döner
}

// Bir string'in belirli bir karakterle başlayıp başlamadığını kontrol eder.
bool c_ampersand_string_starts_with(const char *str, char c) {
    return str != NULL && *str == c; // str null değilse ve ilk karakter c ise true
}

// Bir string'in belirli bir alt string ile başlayıp başlamadığını kontrol eder.
bool c_ampersand_string_starts_with_string(const char *str, const char *prefix) {
    return str != NULL && prefix != NULL && strncmp(str, prefix, strlen(prefix)) == 0; // strncmp ilk n karakteri karşılaştırır
}

// Bir string'in belirli bir karakterle bitip bitmediğini kontrol eder.
bool c_ampersand_string_ends_with(const char *str, char c) {
    if (str == NULL) return false;
    size_t len = strlen(str);
    return len > 0 && str[len - 1] == c; // uzunluk > 0 ise son karaktere bak
}

// Bir string'in belirli bir alt string ile bitip bitmediğini kontrol eder.
bool c_ampersand_string_ends_with_string(const char *str, const char *suffix) {
    if (str == NULL || suffix == NULL) return false;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    // Stringin uzunluğu alt stringden küçükse bitmesi mümkün değil
    return str_len >= suffix_len && strcmp(str + (str_len - suffix_len), suffix) == 0; // son kısmını karşılaştır
}

// Bir string içindeki bir karakterin ilk konumunu bulur. Bulunamazsa -1 döner.
int c_ampersand_string_index_of_char(const char *str, char c) {
    if (str == NULL) return -1; // NULL stringde karakter bulunamaz
    char *ptr = strchr(str, c);
    if (ptr == NULL) {
        return -1; // Karakter bulunamadı
    }
    return (int)(ptr - str); // Pointer farkı indexi verir
}

// Bir string içindeki bir alt string'in ilk konumunu bulur. Bulunamazsa -1 döner.
int c_ampersand_string_index_of_string(const char *str, const char *substring) {
     if (str == NULL || substring == NULL) return -1; // NULL string/substringde arama yapılamaz
     if (strlen(substring) == 0) return 0; // Boş stringin ilk geçtiği yer başlangıçtır
    char *ptr = strstr(str, substring);
    if (ptr == NULL) {
        return -1; // Alt string bulunamadı
    }
    return (int)(ptr - str); // Pointer farkı indexi verir
}

// Bir tam sayıyı string'e çevirir ve sonucu hedef buffer'a yazar.
c_ampersand_result c_ampersand_string_from_int(char *dest, int value) {
    // Parametre doğrulama
    if (dest == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_from_int: Hedef buffer NULL.");
    }
     // NOT: Hedef buffer'ın yeterli boyutta olduğundan emin olma sorumluluğu çağırandadır.
     // Max int değeri için yeterli alan (örn. -2147483648 için ~11 karakter + null = 12 byte) düşünülmelidir.
     // snprintf kullanmak sprintf'ten daha güvenlidir (buffer taşmasını engeller).
    int chars_written = snprintf(dest, /* buffer boyutu çağırandan bilinmeli veya varsayılmalı */ 12, "%d", value); // int için max 12 char yeterli

    // snprintf hatası veya taşma kontrolü
    if (chars_written < 0 || chars_written >= 12 /* veya dest boyutu */) {
        // Formatlama hatası veya buffer taşması
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_IO_ERROR, "string_from_int: Sayı stringe çevrilirken formatlama hatası veya buffer yetersiz.");
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir string'i tam sayıya çevirir.
// str: Tam sayıya çevrilecek string.
// out_value: Çevrilen tam sayının yazılacağı pointer. Başarılı olursa doldurulur.
// Dönüş: Başarı (C_AMPERSAND_OK) veya hata (C_AMPERSAND_ERROR_INVALID_ARGUMENT, C_AMPERSAND_ERROR_INVALID_FORMAT).
c_ampersand_result c_ampersand_string_to_int(const char *str, int *out_value) {
    // Parametre doğrulama
    if (str == NULL || out_value == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "string_to_int: Geçersiz argüman (NULL pointer).");
    }

    // Standart C atoi Sahne64 toolchain'i tarafından sağlanıyorsa kullanılabilir.
    // Ancak atoi hata durumunu belirtmez. Daha güvenli fonksiyonlar (strtol) veya özel parsing gerekir.
    // strtol Sahne toolchain'de varsa:
     char *endptr;
     long val = strtol(str, &endptr, 10);
     if (endptr == str || *endptr != '\0' || (val == LONG_MIN || val == LONG_MAX) && errno == ERANGE) {
    //     // Parsing hatası veya taşma
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_FORMAT, "string_to_int: String tam sayı formatında değil veya taşma.");
     }
    // // Taşma kontrolü int sınırlarına göre de yapılmalı
         if (val < INT_MIN || val > INT_MAX) {
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_FORMAT, "string_to_int: String int sınırlarının dışında.");
     }
     *out_value = (int)val;

    // Atoi kullanmak (hata kontrolü olmadan):
     int value = atoi(str);
     *out_value = value;
     return C_AMPERSAND_OK_RESULT; // Başarı (Hata varsa 0 döndürebilir)

    // Manuel veya basit bir parsing implementasyonu (no_std için):
    // İşaret kontrolü, rakamları oku, basamak değerlerini hesapla.
    // Örneğin:
    int value = 0;
    bool is_negative = false;
    const char *p = str;

    // Boşlukları atla
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }

    // İşaret kontrolü
    if (*p == '-') {
        is_negative = true;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // En az bir rakam olmalı
    if (*p == '\0' || (*p < '0' || *p > '9')) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_FORMAT, "string_to_int: String geçerli tam sayı formatında değil.");
    }

    // Rakamları oku
    while (*p >= '0' && *p <= '9') {
        // Taşma kontrolü yapılmalıdır!
        // Mevcut basit implementasyonda taşma göz ardı ediliyor.
        value = value * 10 + (*p - '0');
        p++;
    }

    // Sonunda sadece null sonlandırıcı olmalı
    if (*p != '\0') {
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_FORMAT, "string_to_int: String tam sayıdan sonra geçersiz karakterler içeriyor.");
    }


    *out_value = is_negative ? -value : value;
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Temel matematik fonksiyonu implementasyonu c&_stdlib.c'ye taşındı.

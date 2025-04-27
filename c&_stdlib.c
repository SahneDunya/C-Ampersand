#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (sahne_* fonksiyonları, sahne_error_t, vsnprintf varsayımı için)
#include "sahne.h"
// C& bellek yönetimi fonksiyonları için (hata mesajı allocate etmek için)
#include "c&_memory.h"

#include <string.h> // strlen, vsnprintf için (varsayım)
#include <stdarg.h> // va_list, va_start, va_end için


// Helper function to map Sahne64 error codes to C& error codes
// Bu fonksiyon, Sahne64 API'sından gelen sahne_error_t değerlerini
// C& dilinin c_ampersand_error_code değerlerine eşler.
c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err) {
    // Bu, Sahne64 API'sının spesifik hata kodlarına bağlı olacaktır.
    // İşte bazı varsayımsal eşlemeler:

    switch (sahne_err) {
        case SAHNE_SUCCESS: return C_AMPERSAND_OK;
        case SAHNE_ERROR_OUT_OF_MEMORY: return C_AMPERSAND_ERROR_OUT_OF_MEMORY;
        case SAHNE_ERROR_INVALID_ARGUMENT: return C_AMPERSAND_ERROR_INVALID_ARGUMENT;
        case SAHNE_ERROR_FILE_NOT_FOUND: return C_AMPERSAND_ERROR_FILE_NOT_FOUND;
        case SAHNE_ERROR_PERMISSION_DENIED: return C_AMPERSAND_ERROR_PERMISSION_DENIED;
        case SAHNE_ERROR_IO_ERROR: return C_AMPERSAND_ERROR_IO_ERROR; // Genel I/O hatası
        case SAHNE_ERROR_RESOURCE_UNAVAILABLE: return C_AMPERSAND_ERROR_RESOURCE_UNAVAILABLE; // Kaynak meşgul/geçici olarak yok
        case SAHNE_ERROR_LIMIT_REACHED: return C_AMPERSAND_ERROR_LIMIT_REACHED; // Limit aşıldı (örn. max dosya sayısı)
        case SAHNE_ERROR_NOT_SUPPORTED: return C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION; // Desteklenmeyen işlem
        // ... Sahne API'sının diğer hata kodları ...
        default: return C_AMPERSAND_ERROR_UNKNOWN; // Bilinmeyen Sahne hatası
    }
}


// Mesaj içeren bir hata c_ampersand_result yapısı oluşturur.
// Message stringi heap'te tahsis edilir.
c_ampersand_result c_ampersand_error_with_message(c_ampersand_result_code code, const char *format, ...) {
    c_ampersand_result error_result;
    error_result.code = code;
    error_result.message = NULL;
    error_result.message_allocated_size = 0;

    if (format == NULL || code == C_AMPERSAND_OK) {
        return error_result; // Mesaj yoksa veya OK durumuysa boş mesajla dön
    }

    // Mesajı formatlamak için gereken boyutu hesapla (vsnprintf'nin C99 özelliği)
    // Sahne64'ün libc implementasyonunda vsnprintf olduğunu varsayıyoruz.
    va_list args;
    va_start(args, format);
    // vsnprintf ile NULL buffer ve 0 size vererek sadece gereken boyutu al (null sonlandırıcı hariç)
    int required_size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (required_size < 0) {
        // vsnprintf hatası, mesaj oluşturulamadı
        // Bu noktada bir hata loglamak en iyisidir.
        // c_ampersand_println("STDLIB HATA: Hata mesajı formatlanırken hata (%d)", required_size);
        return error_result; // Boş mesajla dön
    }

    // Mesaj stringi için bellek tahsis et (null sonlandırıcı dahil)
    size_t allocation_size = (size_t)required_size + 1;
    char *message_buffer = NULL;
    c_ampersand_result alloc_res = c_ampersand_allocate(allocation_size, (void**)&message_buffer);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği hatası, allocate içinde raporlanır.
        // Orijinal hatayı döndüremeyiz (çünkü bu fonksiyonun çıktısı).
        // Bu noktada sadece bellek hatasıyla bir result dönelim (mesajı olmayacak).
         c_ampersand_println("STDLIB HATA: Hata mesajı için bellek yetersiz.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0};
    }

    // Mesajı tekrar formatla, bu sefer tahsis edilen buffera yaz
    va_start(args, format);
    vsnprintf(message_buffer, allocation_size, format, args);
    va_end(args);

    // Result yapısını doldur
    error_result.message = message_buffer;
    error_result.message_allocated_size = allocation_size;

    return error_result; // Mesajlı hata result'ı dön
}

// Bir c_ampersand_result yapısındaki hata mesajı stringini serbest bırakır.
c_ampersand_result c_ampersand_result_free_message(c_ampersand_result *result) {
    if (result == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL pointer güvenli
    }

    if (result->message != NULL && result->message_allocated_size > 0) {
        // Mesaj stringini serbest bırak
        c_ampersand_result free_res = c_ampersand_free(result->message, result->message_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
            // Bellek serbest bırakma hatası
             c_ampersand_println("STDLIB HATA: Hata mesajı belleği serbest bırakılırken hata (%d)", free_res.code);
             c_ampersand_result_print_error(free_res);
             c_ampersand_result_free(&free_res); // Free mesajının mesajını serbest bırak? Tehlikeli döngü. Loglamak yeterli.
             return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0}; // Kaba bir hata raporu
        }
        result->message = NULL; // Pointer'ı NULL yap
        result->message_allocated_size = 0; // Boyutu sıfırla
    }
    // NOT: Result struct'ının kendisi bu fonksiyon tarafından serbest bırakılmaz.

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Formatsız stringi standart çıktıya yazdırır.
// Sahne64 konsol API'sını kullanır.
c_ampersand_result c_ampersand_print(const char *str) {
    if (str == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    size_t len = strlen(str);
    if (len == 0) {
        return C_AMPERSAND_OK_RESULT; // Boş string yazdırmak başarıdır
    }

    // Sahne64 konsol API'sını kullanarak yazdır
    // Varsayım: sahne_console_write sahne.h'de tanımlı, sahne_error_t döner ve yazılan byte sayısını out parametresine yazar.
    size_t bytes_written;
    sahne_error_t sahne_err = sahne_console_write(str, len, &bytes_written); // stdout varsayımı

    if (sahne_err != SAHNE_SUCCESS) {
         // Sahne konsol yazma hatası
        c_ampersand_error_code camper_err_code = map_sahne_error_to_camper_error(sahne_err);
        return c_ampersand_error_with_message(camper_err_code, "Konsola yazma hatası (SAHNE hatası: %d)", sahne_err);
    }

    // Tamamı yazılmadıysa ne yapılmalı? (Kuyrukta bekletme, hata dönme?)
    // Şimdilik tamamının yazıldığını varsayalım veya bu durumu hata olarak raporlayalım.
    if (bytes_written != len) {
         // Tamamı yazılamadı, potansiyel hata veya buffer dolu
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_IO_ERROR, "Konsola yazma hatası: Tamamı yazılamadı (%zu / %zu)", bytes_written, len);
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Formatlı stringi standart çıktıya yazdırır ve sonuna yeni satır ekler.
// vsnprintf ve Sahne64 konsol API'sını kullanır.
c_ampersand_result c_ampersand_println(const char *format, ...) {
     if (format == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Formatlı mesajı bir buffera yazdır. Boyutu önceden belirlemek zor.
    // Dinamik buffer kullanmak daha sağlamdır, ancak sabit buffer ile başlayalım.
    // Alternatif: İki vsnprintf çağrısı ile boyutu bulup allocate etmek.
    char print_buffer[512]; // Sabit boyutlu buffer (daha sağlam implementasyon dinamik kullanır)

    va_list args;
    va_start(args, format);
    // vsnprintf'nin Sahne/C& stdlib'de mevcut olduğunu varsayalım.
    int chars_written = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
    va_end(args);

    if (chars_written < 0 || (size_t)chars_written >= sizeof(print_buffer)) {
        // vsnprintf hatası veya buffer taşması
        // Hata loglamak veya hata result'ı döndürmek gerekli.
        // Bu fonksiyon genellikle debug/bilgi amaçlı kullanıldığı için loglamak yeterli olabilir.
         c_ampersand_println("STDLIB HATA: println formatlanırken hata veya buffer taşması."); // DİKKAT: Bu rekürsif çağrı olabilir!
         // Alternatif olarak, bu hata durumunu belirtmek için sadece C_AMPERSAND_ERROR_IO_ERROR gibi bir result döndürülür.
         return (c_ampersand_result){C_AMPERSAND_ERROR_IO_ERROR, NULL, 0};
    }

    // Formatlanmış mesajı yazdır (print fonksiyonunu kullan)
    c_ampersand_result print_res = c_ampersand_print(print_buffer);
    if (print_res.code != C_AMPERSAND_OK) {
        // print sırasında hata oldu, hatayı ilet
        // print_res zaten mesaj içerebilir.
        return print_res;
    }

    // Yeni satır karakterini yazdır
    c_ampersand_result newline_res = c_ampersand_print("\n");
     if (newline_res.code != C_AMPERSAND_OK) {
        // Yeni satır yazma hatası
        return newline_res;
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}


// Temel matematik fonksiyonu
int c_ampersand_abs(int n) {
    // Standart C abs fonksiyonu (varsayım: Sahne toolchain sağlıyor)
    return n < 0 ? -n : n;
}
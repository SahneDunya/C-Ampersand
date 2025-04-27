#include "c&_error_handling.h"
// c_ampersand_result yapısı, C_AMPERSAND_* hata kodları, C_AMPERSAND_OK_RESULT ve c_ampersand_println için
#include "c&_stdlib.h"
// SAHNE64 C API başlığı (Bellek yönetimi ve hata tipleri için)
// #include "sahne64_api.h" // Eski isim
#include "sahne.h"

#include <stdarg.h> // Değişken argümanlar (va_list vb.) için
#include <string.h> // strlen, memcpy için (formatlanmış mesaj kopyalama)
#include <stddef.h> // size_t için (gerekmeyebilir, sahne.h veya c&_stdlib.h sağlar)


// Varsayım: c_ampersand_result structı { c_ampersand_error_code code; char *message; size_t message_size; } şeklindedir (c&_stdlib.h'de tanımlı).
// Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de const olarak tanımlı (Genellikle { C_AMPERSAND_OK, NULL, 0 } şeklinde).
// Varsayım: C_AMPERSAND_* hata kodu enum'ı c&_error_handling.h'de tanımlı (artık yukarıya ekledik).
// Varsayım: c_ampersand_println fonksiyonu c&_stdlib.h'de bildirilmiş ve başka bir yerde SAHNE64 G/Ç'si kullanarak implemente edilmiştir.
// Varsayım: vsnprintf veya benzeri bir formatlama fonksiyonu toolchain tarafından sağlanıyor veya başka bir C& modülünde mevcut.

c_ampersand_result c_ampersand_ok() {
    // C_AMPERSAND_OK_RESULT genellikle { C_AMPERSAND_OK, NULL, 0 } olarak tanımlanır.
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_error(c_ampersand_error_code code) {
    // Hata kodu ve NULL mesaj ile bir sonuç oluştur
    return (c_ampersand_result){code, NULL, 0}; // message_size da 0 olmalı
}

// Belirli bir hata kodu ve formatlanmış mesajıyla bir hata sonucu döndürür.
// Mesaj stringi SAHNE64 belleğinde ayrılır.
c_ampersand_result c_ampersand_error_with_message(c_ampersand_error_code code, const char *message_format, ...) {
    // Hata sonucu için başlangıç değeri (mesajsız hata gibi)
    c_ampersand_result result = {code, NULL, 0};

    if (message_format != NULL) {
        va_list args;
        int needed_size;

        va_start(args, message_format);
        // vsnprintf kullanarak formatlanmış mesaj için gerekli boyutu hesapla
        // Varsayım: vsnprintf no_std ortamında mevcut ve NULL buffer ile boyut hesaplayabiliyor.
        needed_size = vsnprintf(NULL, 0, message_format, args);
        va_end(args);

        // vsnprintf hata döndürürse (negatif) veya mesaj boşsa
        if (needed_size < 0) {
             // Formatlama hatası veya boş mesaj durumu
             // Bellek tahsisi yapmaya gerek yok, mesajsız hata gibi davranabilir.
             // Veya daha spesifik bir hata kodu dönebiliriz.
             result.code = C_AMPERSAND_ERROR_GENERIC; // Genel bir hata kodu
             return result;
        }

        // vsnprintf boyutu null sonlandırıcı olmadan verir, bu yüzden +1 ekliyoruz.
        size_t allocation_size = (size_t)needed_size + 1;
        char *formatted_message = NULL;

        // Mesaj stringi için SAHNE64 belleği tahsis et
        sahne_error_t sahne_alloc_err = sahne_mem_allocate(allocation_size, (void**)&formatted_message);

        if (sahne_alloc_err != SAHNE_SUCCESS) {
            // Bellek tahsis hatası
            // Orijinal hata kodunu kaybediyoruz, ya hata kodu enumuna SAHNE_OOM gibi bir şey eklemeliydik
            // ya da burada loglayıp OOM hatası dönmeliyiz.
            // Orijinal hata kodunu koruyarak OOM hatası dönelim.
            // Varsayım: C_AMPERSAND_ERROR_OUT_OF_MEMORY c&_stdlib.h'de tanımlı
            result.code = C_AMPERSAND_ERROR_OUT_OF_MEMORY; // OOM hatası kodunu ayarla
            result.message = NULL; // Mesaj NULL kalır
            result.message_size = 0;
            return result;
        }

        // Tahsis başarılı, formatlanmış mesajı belleğe yaz
        va_start(args, message_format); // vsnprintf tekrar çağrılacağı için args'ı sıfırla
        vsnprintf(formatted_message, allocation_size, message_format, args); // allocation_size = needed_size + 1
        va_end(args);

        // vsnprintf her zaman null sonlandırıcı ekler (buffer yeterliyse)
        // allocated_size, null sonlandırıcıyı içerdiği için boyut doğru.

        // Sonucu ayarla
        result.code = code; // Orijinal hata kodunu kullan
        result.message = formatted_message;
        result.message_size = allocation_size; // Ayrılan tam boyutu kaydet

    } // message_format NULL ise, result zaten {code, NULL, 0} olarak başlatıldı.

    return result;
}

// Bir sonucun hata içerip içermediğini kontrol eder.
bool c_ampersand_result_is_error(c_ampersand_result result) {
    return result.code != C_AMPERSAND_OK;
}

// Bir sonuçtan hata kodunu alır.
c_ampersand_error_code c_ampersand_result_get_code(c_ampersand_result result) {
    return result.code;
}

// Bir sonuçtan hata mesajını alır (varsa).
const char *c_ampersand_result_get_message(c_ampersand_result result) {
    return result.message;
}

// Bir hata sonucunu C& çıktı mekanizmasını kullanarak yazdırır.
// SAHNE64 kaynak G/Ç'sini kullanır (c_ampersand_println aracılığıyla).
void c_ampersand_result_print_error(c_ampersand_result result) {
    // c_ampersand_println'in printf benzeri formatlama alabildiğini varsayıyoruz.
    // Aksi takdirde, burada formatlanmış mesajı bir buffera alıp sonra c_ampersand_println ile yazdırmak gerekirdi.
    if (c_ampersand_result_is_error(result)) {
        c_ampersand_println("Hata Kodu: %d", result.code); // Hata kodunu yazdır
        if (result.message != NULL) {
            c_ampersand_println("Mesaj: %s", result.message); // Mesajı yazdır
        }
        // c_ampersand_println zaten satır sonu ekleyebilir veya ayrıca ekleyebiliriz.
        // c_ampersand_println("\n"); // İsteğe bağlı: Ekstra satır sonu
    }
}

// Bir c_ampersand_result yapısı içindeki hata mesajı belleğini serbest bırakır (varsa).
// SAHNE64 bellek serbest bırakmayı kullanır.
void c_ampersand_result_free(c_ampersand_result *result) {
    // Pointer geçerli mi, mesaj pointer'ı null değil mi ve boyutu sıfırdan büyük mü kontrol et
    if (result != NULL && result->message != NULL && result->message_size > 0) {
        // Mesaj belleğini SAHNE64 API kullanarak serbest bırak
        // sahne_error_t sahne_free_err =
        sahne_mem_release(result->message, result->message_size);
        // Serbest bırakma hatası durumunda loglama yapılabilir.

        // Pointer ve boyutu sıfırla
        result->message = NULL;
        result->message_size = 0;
    }
    // Eğer result zaten NULL ise veya mesaj yoksa, işlem yapma.
}


// Örnek bir hata mesajı alma fonksiyonu (Sahne64 API'sından)
// Sahne64'ün hata kodlarını C& stringlerine çevirmek için yardımcı olabilir.
const char *c_ampersand_get_sahne_error_message(sahne_error_t sahne_err_code) {
    // Bu sadece bir örnektir, gerçek implementasyon Sahne64 API'sına bağlıdır.
    switch (sahne_err_code) {
        case SAHNE_ERROR_OUT_OF_MEMORY:
            return "Sahne64: Bellek yetersiz.";
        case SAHNE_ERROR_INVALID_PARAMETER:
            return "Sahne64: Geçersiz parametre.";
        case SAHNE_ERROR_RESOURCE_NOT_FOUND:
             return "Sahne64: Kaynak bulunamadı.";
        case SAHNE_ERROR_PERMISSION_DENIED:
             return "Sahne64: Yetki reddedildi.";
        case SAHNE_ERROR_RESOURCE_BUSY:
             return "Sahne64: Kaynak meşgul.";
        case SAHNE_ERROR_INVALID_HANDLE:
             return "Sahne64: Geçersiz Handle.";
        // ... diğer Sahne64 hata kodları için mesajlar
        default:
            // Bilinmeyen Sahne64 hata kodları için daha genel bir mesaj.
             // Tüm SAHNE hata kodlarını map etmek idealdir.
            return "Sahne64: Bilinmeyen hata.";
    }
}

c_ampersand_result c_ampersand_error_from_sahne_error(sahne_error_t sahne_err) {
    c_ampersand_error_code cam_code;
    const char* base_message = c_ampersand_get_sahne_error_message(sahne_err); // Sahne mesajını al

    // Sahne hatasını C& hatasına eşle
    switch(sahne_err) {
        case SAHNE_SUCCESS: // Başarı durumu hata fonksiyonuna gelmemeli
            return c_ampersand_ok();
        case SAHNE_ERROR_OUT_OF_MEMORY:
            cam_code = C_AMPERSAND_ERROR_OUT_OF_MEMORY;
            break;
        case SAHNE_ERROR_INVALID_PARAMETER:
            cam_code = C_AMPERSAND_ERROR_INVALID_ARGUMENT;
            break;
        case SAHNE_ERROR_RESOURCE_NOT_FOUND:
             cam_code = C_AMPERSAND_ERROR_IO; // Veya daha spesifik bir C& I/O hatası
             break;
        // ... diğer eşlemeler ...
        default:
            cam_code = C_AMPERSAND_ERROR_GENERIC; // Eşleşmeyenler için genel hata
            break;
    }

    // Formatlanmış mesaj oluştur (isteğe bağlı, sadece temel mesajı da kullanabiliriz)
    // return c_ampersand_error_with_message(cam_code, "SAHNE64 Hatası: %s", base_message);
    // Basitçe base mesajı kullanmak için:
    // Bu durumda c_ampersand_result'ın message alanının const char* olmaması ve
    // bu stringin yaşam döngüsünün yönetilmesi gerekir (örn. static string veya kopyalama)
    // Kopyalamak için tekrar sahne_mem_allocate kullanmak gerekir.
    size_t msg_len = strlen(base_message);
    size_t alloc_size = msg_len + 1;
    char* msg_copy = NULL;
    if (sahne_mem_allocate(alloc_size, (void**)&msg_copy) == SAHNE_SUCCESS) {
        memcpy(msg_copy, base_message, alloc_size);
        return (c_ampersand_result){cam_code, msg_copy, alloc_size};
    } else {
        // Mesajı kopyalayamıyorsak OOM hatası dön
         return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0};
    }
}

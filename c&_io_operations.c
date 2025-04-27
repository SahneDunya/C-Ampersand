#include "c&_io_operations.h"
// c_ampersand_result yapısı, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle, u64, sahne_error_t, SAHNE_ERROR_*, MODE_* için)
#include "sahne.h"

#include <string.h> // strlen için
#include <stddef.h> // size_t, NULL için
#include <stdint.h> // uint32_t için


// Standart G/Ç Handle'ları (dışarıdan atanması gereken global değişkenler)
u64 c_ampersand_stdin_handle = 0;  // Genellikle C& runtime başlangıcında ayarlanır
u64 c_ampersand_stdout_handle = 0; // Genellikle C& runtime başlangıcında ayarlanır
u64 c_ampersand_stderr_handle = 0; // Genellikle C& runtime başlangıcında ayarlanır

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: C_AMPERSAND_* hata kodları c&_stdlib.h veya c&_error_handling.h'de tanımlı
static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err) {
    switch (sahne_err) {
        case SAHNE_SUCCESS: return C_AMPERSAND_OK; // Başarı durumu asla bu fonksiyona gelmemeli
        case SAHNE_ERROR_OUT_OF_MEMORY: return C_AMPERSAND_ERROR_OUT_OF_MEMORY;
        case SAHNE_ERROR_INVALID_PARAMETER: return C_AMPERSAND_ERROR_INVALID_ARGUMENT;
        case SAHNE_ERROR_RESOURCE_NOT_FOUND: return C_AMPERSAND_ERROR_FILE_NOT_FOUND; // Dosya bulunamadı
        case SAHNE_ERROR_PERMISSION_DENIED: return C_AMPERSAND_ERROR_PERMISSION_DENIED;
        case SAHNE_ERROR_RESOURCE_BUSY: return C_AMPERSAND_ERROR_IO; // Daha spesifik bir IO hatası
        case SAHNE_ERROR_INTERRUPTED: return C_AMPERSAND_ERROR_IO; // Kesilme
        case SAHNE_ERROR_INVALID_OPERATION: return C_AMPERSAND_ERROR_IO; // Okuma/yazma hatası gibi
        case SAHNE_ERROR_NOT_SUPPORTED: return C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION;
        case SAHNE_ERROR_INVALID_HANDLE: return C_AMPERSAND_ERROR_INVALID_ARGUMENT; // Geçersiz Handle
        // ... diğer SAHNE_ERROR_* kodları için eşlemeler ...
        default: return C_AMPERSAND_ERROR_IO; // Bilinmeyen Sahne hataları için genel IO hatası
    }
}


// Kaynak/dosya açar (Sahne64 resource_acquire'a karşılık gelir). Path Sahne64 Resource ID formatında olmalı.
// Handle'ı *fd parametresine yazar.
c_ampersand_result c_ampersand_io_open(const char *path, uint32_t flags, c_ampersand_file_handle *handle) {
    if (path == NULL || handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    u64 acquired_handle = C_AMPERSAND_INVALID_FILE_HANDLE; // Alınan Handle için değişken
    // Sahne64 resource_acquire fonksiyonu sahne_error_t döner ve Handle'ı out_handle* parametresine yazar.
    // Path için char* ve size_t alır, biz strlen kullanıyoruz.
    sahne_error_t sahne_err = sahne_resource_acquire(path, strlen(path), flags, &acquired_handle);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        *handle = C_AMPERSAND_INVALID_FILE_HANDLE; // Hata durumunda Handle'ı geçersiz yap
        // Varsayım: c_ampersand_result { code, message*, message_size } yapısında, message NULL olabilir.
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    // Başarılı olursa döndürülen Handle'ı C& file handle'ına ata
    *handle = acquired_handle;
    // Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de tanımlı ({C_AMPERSAND_OK, NULL, 0})
    return C_AMPERSAND_OK_RESULT;
}

// Kaynağı kapatır/serbest bırakır (Sahne64 resource_release'e karşılık gelir).
c_ampersand_result c_ampersand_io_close(c_ampersand_file_handle handle) {
    // handle bir Sahne64 Handle'ı (u64)
    if (handle == C_AMPERSAND_INVALID_FILE_HANDLE) { // Geçersiz Handle
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Zaten kapalı veya geçersiz
    }

    // Sahne64 resource_release fonksiyonu sahne_error_t döner.
    sahne_error_t sahne_err = sahne_resource_release(handle);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Kaynaktan veri okur (Sahne64 resource_read'e karşılık gelir).
// Okunan byte sayısını *bytes_read parametresine yazar.
c_ampersand_result c_ampersand_io_read(c_ampersand_file_handle handle, void *buffer, size_t size, size_t *bytes_read) {
     // handle bir Sahne64 Handle'ı (u64)
    if (buffer == NULL || bytes_read == NULL || size == 0) { // size 0 ise okumaya gerek yok, ama buffer/bytes_read null olmamalı
         if (bytes_read != NULL) *bytes_read = 0; // size 0 ise okunan byte sayısı 0
         // buffer null veya bytes_read null ise hata
         if (buffer == NULL || bytes_read == NULL) return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
         // size 0 ise okuma yapma ama başarı dön
         return C_AMPERSAND_OK_RESULT;
    }
    if (handle == C_AMPERSAND_INVALID_FILE_HANDLE) { // Geçersiz Handle
         *bytes_read = 0;
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Geçersiz dosya tanımlayıcısı
    }

    size_t actual_bytes_read = 0;
    // Sahne64 resource_read fonksiyonu sahne_error_t döner ve okunan byte sayısını out_bytes_read* parametresine yazar.
    sahne_error_t sahne_err = sahne_resource_read(handle, buffer, size, &actual_bytes_read);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
         *bytes_read = 0; // Hata durumunda okunan byte sayısı 0
         return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    // Başarılı olursa okunan byte sayısını çıktı parametresine yaz
    *bytes_read = actual_bytes_read;

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Kaynağa yazar (Sahne64 resource_write'a karşılık gelir).
// Yazılan byte sayısını *bytes_written parametresine yazar.
c_ampersand_result c_ampersand_io_write(c_ampersand_file_handle handle, const void *buffer, size_t size, size_t *bytes_written) {
     // handle bir Sahne64 Handle'ı (u64)
     if (buffer == NULL || bytes_written == NULL || size == 0) { // size 0 ise yazmaya gerek yok, ama buffer/bytes_written null olmamalı
         if (bytes_written != NULL) *bytes_written = 0; // size 0 ise yazılan byte sayısı 0
         // buffer null veya bytes_written null ise hata
         if (buffer == NULL || bytes_written == NULL) return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
         // size 0 ise yazma yapma ama başarı dön
         return C_AMPERSAND_OK_RESULT;
     }
     if (handle == C_AMPERSAND_INVALID_FILE_HANDLE) { // Geçersiz Handle
         *bytes_written = 0;
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Geçersiz dosya tanımlayıcısı
     }


    size_t actual_bytes_written = 0;
    // Sahne64 resource_write fonksiyonu sahne_error_t döner ve yazılan byte sayısını out_bytes_written* parametresine yazar.
    sahne_error_t sahne_err = sahne_resource_write(handle, buffer, size, &actual_bytes_written);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
         *bytes_written = 0; // Hata durumunda yazılan byte sayısı 0
         return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    // Başarılı olursa yazılan byte sayısını çıktı parametresine yaz
    *bytes_written = actual_bytes_written;

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Standart çıktıya bir string yazar (stdout Handle'ını kullanır).
c_ampersand_result c_ampersand_io_print(const char *message) {
     if (message == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
     }
     // Standart çıktı Handle'ı geçerli mi kontrol et
     if (c_ampersand_stdout_handle == C_AMPERSAND_INVALID_FILE_HANDLE) {
         // stdout Handle'ı ayarlanmamış (runtime başlatma hatası?)
         // Bu durumda çıktı veremeyiz.
         return (c_ampersand_result){C_AMPERSAND_ERROR_IO, NULL, 0}; // Veya daha spesifik bir hata kodu
     }

    size_t len = c_ampersand_string_length(message);
    size_t bytes_written = 0; // c_ampersand_io_write çıktısı

    // Standart çıktı Handle'ına yaz
    // c_ampersand_io_write zaten hata durumunu c_ampersand_result olarak döndürüyor.
    return c_ampersand_io_write(c_ampersand_stdout_handle, message, len, &bytes_written);
}

// Standart çıktıya bir string yazar ve satır sonu ekler (stdout Handle'ını kullanır).
c_ampersand_result c_ampersand_io_println(const char *message) {
     // Stringi yazdır
    c_ampersand_result result = c_ampersand_io_print(message);

    // String yazdırma başarılıysa satır sonu karakterini yazdır
    // İlk işlem başarısızsa ikinciyi denemeden ilk hatayı dönmeliyiz.
    if (result.code == C_AMPERSAND_OK) {
        result = c_ampersand_io_print("\n"); // "\n" stringini yazdır
    }
    return result;
}

// Bir kaynaktan (stdin veya dosya) bir satır okur.
// handle: Okunacak kaynağın Handle'ı (stdin_handle, dosya handle'ı vb.)
// buffer: Okunan satırın yazılacağı buffer
// buffer_size: Buffer'ın boyutu
// bytes_read: Okunan toplam byte sayısı (null sonlandırıcı dahil)
// Varsayım: Sahne64 API'si resource_read_line benzeri bir fonksiyon sağlar.
c_ampersand_result c_ampersand_io_read_line(c_ampersand_file_handle handle, char *buffer, size_t buffer_size, size_t *bytes_read) {
     // Handle, buffer, bytes_read null olmamalı, buffer_size 0 olmamalı
     if (handle == C_AMPERSAND_INVALID_FILE_HANDLE || buffer == NULL || bytes_read == NULL || buffer_size == 0) {
          if (bytes_read != NULL) *bytes_read = 0;
          return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
     }

    size_t actual_bytes_read = 0;
    // Varsayım: sahne_resource_read_line fonksiyonu sahne.h'de tanımlı,
    // sahne_error_t döner ve okunan byte sayısını *out_bytes_read'e yazar.
    // Bu fonksiyonun satır sonunu okuyup null sonlandırıcı eklediği varsayılıyor.
    // Eğer böyle bir Sahne API fonksiyonu yoksa, burada resource_read ile byte byte okuma loop'u implemente edilmelidir.
    sahne_error_t sahne_err = sahne_resource_read_line(handle, buffer, buffer_size, &actual_bytes_read);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        *bytes_read = 0; // Hata durumunda okunan byte sayısı 0
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    // Başarılı olursa okunan byte sayısını çıktı parametresine yaz
    *bytes_read = actual_bytes_read;

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}


// Basit bir string uzunluğu fonksiyonu (c&_string.c'de daha gelişmiş bir sürümü olabilir)
size_t c_ampersand_string_length(const char *str) {
    // NULL pointer kontrolü ekleyelim
     if (str == NULL) return 0;

    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

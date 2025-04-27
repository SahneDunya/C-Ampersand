#include "c&_memory.h"
// c_ampersand_result yapısı, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT için
#include "c&_stdlib.h"
#include "sahne.h"

#include <stddef.h> // size_t, NULL için

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: C_AMPERSAND_* hata kodları c&_stdlib.h veya c&_error_handling.h'de tanımlı
static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err) {
    switch (sahne_err) {
        case SAHNE_SUCCESS: return C_AMPERSAND_OK; // Başarı durumu asla bu fonksiyona gelmemeli
        case SAHNE_ERROR_OUT_OF_MEMORY: return C_AMPERSAND_ERROR_OUT_OF_MEMORY; // Bellek yetersiz
        case SAHNE_ERROR_INVALID_PARAMETER: return C_AMPERSAND_ERROR_INVALID_ARGUMENT; // Geçersiz parametre (örn. yanlış pointer/boyut)
        // ... diğer SAHNE_ERROR_* kodları için eşlemeler ...
        default: return C_AMPERSAND_ERROR_GENERIC; // Bilinmeyen Sahne hataları için genel hata
    }
}


// Belirli boyutta bellek ayırır.
// Bellek Sahne64 API'si kullanılarak tahsis edilir.
c_ampersand_result c_ampersand_allocate(size_t size, void **ptr) {
    if (ptr == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // ptr NULL olamaz
    }

    // Sahne64'ten bellek talep et
    void *allocated_memory = NULL; // sahne_mem_allocate bu pointer'ı dolduracak
    sahne_error_t sahne_err = sahne_mem_allocate(size, &allocated_memory);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Hata durumunda ptr'yi NULL yap
        *ptr = NULL;
        // SAHNE hatasını C& hatasına çevirerek dön
        // Varsayım: c_ampersand_result { code, message*, message_size } yapısında, message NULL olabilir.
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    // Başarılı olursa tahsis edilen pointer'ı çıktı parametresine ata
    *ptr = allocated_memory;
    // Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de tanımlı ({C_AMPERSAND_OK, NULL, 0})
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Daha önce c_ampersand_allocate ile ayrılmış olan belleği serbest bırakır.
// Bellek Sahne64 API'si kullanılarak serbest bırakılır.
// NOT: ptr NULL ise veya size 0 ise (ve ptr NULL değilse) davranış tanımlıdır.
// Standart free(NULL) no-op olduğu için burada da NULL ptr için başarı döneceğiz.
// ptr NULL değil ama size 0 ise, bu Sahne API'si için geçersiz parametre olabilir.
c_ampersand_result c_ampersand_free(void *ptr, size_t size) {
    // ptr NULL ise, standart C davranışı gereği başarı dön (no-op)
    if (ptr == NULL) {
        return C_AMPERSAND_OK_RESULT;
    }

    // ptr NULL değil ama size 0 ise bu muhtemelen bir programlama hatasıdır.
    // SAHNE API'si bu durumda ne yapar bilinmez, INVALID_PARAMETER dönebilir.
    // C& seviyesinde bunu bir hata olarak raporlamak mantıklıdır.
    if (size == 0) {
        // ptr NULL değil ama size 0 ise hata
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Belleği Sahne64'ten serbest bırak
    sahne_error_t sahne_err = sahne_mem_release(ptr, size);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // SAHNE hatasını C& hatasına çevirerek dön
        // Bellek serbest bırakma hatası genellikle ciddi bir durumdur (örn. heap bozulması, geçersiz pointer)
        // Bu durumda C& seviyesinde uygun bir hata kodu dönülmeli.
        return (c_ampersand_result){map_sahne_error_to_camper_error(sahne_err), NULL, 0};
    }

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// ... diğer bellek yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, c_ampersand_reallocate)
// c_ampersand_reallocate implementasyonu da Sahne64'ün reallocate API'si varsa onu kullanır,
// yoksa allocate, copy, free adımlarını manuel yapar.

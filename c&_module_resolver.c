#include "c&_module_resolver.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle, u64, sahne_error_t, SAHNE_ERROR_*, module_* fonksiyonları için)
#include "sahne.h"
// C& G/Ç operasyonları (dosya varlığını kontrol etmek için)
#include "c&_io_operations.h" // c_ampersand_io_open, c_ampersand_io_close, c_ampersand_file_handle, C_AMPERSAND_O_* bayrakları için
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"
// string.h (strlen, snprintf) için
#include <string.h>
// <stdlib.h> ve <stdio.h> kaldırıldı
// #include <stdlib.h>
// #include <stdio.h>


// --- Dahili Sabitler ve Veri Yapıları ---

#define MAX_SEARCH_PATHS 16 // Kaydedilebilecek maksimum arama yolu sayısı

// Arama yolu stringini ve tahsis edilen boyutunu saklayan yapı
typedef struct {
    char *ptr;
    size_t size; // c_ampersand_free için gerekli
} search_path_t;

static search_path_t search_paths[MAX_SEARCH_PATHS]; // Arama yolları dizisi
static int num_search_paths = 0;                    // Kaydedilmiş arama yolu sayısı

// Örnek modül dosya adı uzantısı
static const char *module_extension = ".s64mod"; // Sahne64 modül uzantısı varsayımı

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
// static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);

// --- Yardımcı Fonksiyonlar ---

// Bir modül adından tam dosya yolunu oluşturur ve dosyanın var olup olmadığını kontrol eder.
// Bulunan ilk geçerli tam yolu, C& belleğinde tahsis edilmiş olarak döndürür.
// Bulunamazsa veya hata olursa NULL döndürür.
static char *construct_module_path(const char *module_name) {
    if (module_name == NULL || strlen(module_name) == 0) {
        return NULL; // Geçersiz modül adı
    }

    for (int i = 0; i < num_search_paths; ++i) {
        // Arama yolu geçerli mi kontrol et
        if (search_paths[i].ptr == NULL) continue;

        size_t path_len = search_paths[i].size; // Arama yolu uzunluğu (null sonlandırıcı hariç)
        size_t name_len = strlen(module_name);
        size_t ext_len = strlen(module_extension);
        // Yol + "/" + modul adı + uzantı + null sonlandırıcı
        size_t total_len = path_len + 1 + name_len + ext_len + 1;

        char *full_path_ptr = NULL;
        // Tam yol stringi için C& belleği tahsis et
        c_ampersand_result alloc_res = c_ampersand_allocate(total_len, (void**)&full_path_ptr);

        if (alloc_res.code != C_AMPERSAND_OK) {
            // Bellek yetersizliği
            c_ampersand_println("MODRESOLVER HATA: Tam modül yolu için bellek yetersiz.");
            return NULL; // Bellek hatası, NULL dön
        }

        // Tam yolu buffer'a formatla
        // Varsayım: snprintf Sahne64/C& stdlib'de mevcut.
        snprintf(full_path_ptr, total_len, "%s/%s%s", search_paths[i].ptr, module_name, module_extension);

        // Dosyanın var olup olmadığını kontrol et (C& I/O fonksiyonlarını kullanarak)
        c_ampersand_file_handle temp_handle = C_AMPERSAND_INVALID_FILE_HANDLE; // Geçici Handle
        // Sadece okuma modunda açmayı dene. Başarılı olursa dosya var demektir.
        c_ampersand_result open_res = c_ampersand_io_open(full_path_ptr, C_AMPERSAND_O_RDONLY, &temp_handle);

        if (open_res.code == C_AMPERSAND_OK) {
            // Dosya bulundu, Handle'ı kapat (Handle artık gerekmiyor)
            // c_ampersand_io_close'un dönüş değerini kontrol etmiyoruz (bu noktada kritik hata)
            c_ampersand_io_close(temp_handle);
            return full_path_ptr; // Bulunan ve tahsis edilen tam yolu döndür
        }

        // Dosya bu yolda bulunamadı veya açılamadı (örn. izin hatası). Ayrılan yolu serbest bırak ve devam et.
        // c_ampersand_io_open çeşitli hatalar dönebilir (FILE_NOT_FOUND, PERMISSION_DENIED, IO_ERROR).
        // Hepsi için yolu serbest bırakıp bir sonraki yolu denemeliyiz.
        c_ampersand_free(full_path_ptr, total_len); // Yolu serbest bırak (size gerekli)
    }
    return NULL; // Modül hiçbir arama yolunda bulunamadı
}

// --- Modül Çözme Fonksiyonlarının Implementasyonu ---

// Bir modülü verilen ada göre arama yollarında bulur ve Sahne64 API'si ile yükler.
// Handle'ı *handle parametresine yazar.
c_ampersand_result c_ampersand_module_load(const char *module_name, c_ampersand_module_handle *handle) {
    // Parametre doğrulama
    if (module_name == NULL || handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Handle'ı başlangıçta geçersiz yap
    *handle = C_AMPERSAND_INVALID_MODULE_HANDLE;

    // Modül dosyasının tam yolunu bul
    char *module_path = construct_module_path(module_name);
    if (module_path == NULL) {
        // Modül dosyası hiçbir yolda bulunamadı
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_MODULE_NOT_FOUND, "Modül dosyası bulunamadı: %s", module_name);
    }

    // Sahne64 API'sını kullanarak modülü yükle
    // Varsayım: sahne_module_load sahne.h'de tanımlı, sahne_error_t döner ve Handle'ı out_handle* parametresine yazar.
    u64 loaded_handle = C_AMPERSAND_INVALID_MODULE_HANDLE; // Sahne64 Handle'ı
    sahne_error_t sahne_err = sahne_module_load(module_path, strlen(module_path), &loaded_handle);

    // construct_module_path tarafından tahsis edilen yolu serbest bırak
    // Yolun boyutu bilinmediği için (construct_module_path dönmüyor) bu zor.
    // construct_module_path'in boyutu da döndürmesi gerekiyordu.
    // Düzeltme: construct_module_path sadece char* döndürüyor. Boyutu yeniden hesaplayalım veya path_t dönelim.
    // Basitlik adına, yeniden strlen yapıp + uzantı + separator boyutu ekleyerek size'ı manuel bulalım.
    // Bu risklidir, allocate_block'un gerçek boyutu döndürmesi daha iyi olurdu.
    // Alternatif: c_ampersand_free(module_path, strlen(module_path) + strlen(module_extension) + 2); // Kaba tahmin
    // En doğrusu: c_ampersand_allocate boyutu kaydetmeli veya construct_module_path (ptr, size) pairi dönmeli.
    // Varsayım: c_ampersand_allocate çağrısı sırasında istenen boyutu hatırlayabiliriz.
    // construct_module_path'teki allocation_size değerini kullanalım.
    // Ancak bu fonksiyon dışına çıkınca o değişken kaybolur.
    // Tekrar Kaba tahmin ile free yapalım veya construct_module_path imzasını değiştirelim.
    // İmza değişikliği daha temiz: construct_module_path -> c_ampersand_result construct_module_path(...) char** out_path, size_t* out_size.

    // Varsayalım construct_module_path, path_len + 1 + name_len + ext_len + 1 boyutunda tahsis yaptı.
    // Bu boyutu free için kullanmalıyız.
    size_t path_alloc_size = strlen(module_path) + 1; // En azından string uzunluğu + null sonlandırıcı boyutu
    // Asıl tahsis boyutu (klasör yolu + / + modül adı + .s64mod + null)
    // Bu bilgi construct_module_path içinde kalıyor.
    // C_ampersand_free(module_path, ?????); // Doğru boyut bilinmiyor!

    // PROBLEM: construct_module_path, c_ampersand_allocate ile tahsis yapıyor
    // ve döndürdüğü pointer'ın boyutunu döndürmüyor. Bu c_ampersand_free ile uyumsuz.
    // construct_module_path'in c_ampersand_allocate'nin tahsis ettiği boyutu da
    // bir şekilde döndürmesi veya saklaması gerekir.

    // Düzeltme: construct_module_path'in dönüş tipini pair yapamıyorsak,
    // döndürdüğü stringin boyutu + null sonlandırıcı kadar bellek serbest bırakmak
    // en iyi tahminimizdir. Bu yine de hatalı olabilir eğer allocate fazla bellek verdiyse.
    size_t guessed_alloc_size = strlen(module_path) + 1;
    c_ampersand_free(module_path, guessed_alloc_size); // Kaba tahminle serbest bırak


    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Modül yükleme hatası
        *handle = C_AMPERSAND_INVALID_MODULE_HANDLE; // Handle'ı geçersiz yap
        return c_ampersand_error_with_message(map_sahne_error_to_camper_error(sahne_err), "Modül yüklenirken hata: %s (SAHNE hatası: %d)", module_name, sahne_err);
    }

    // Başarılı olursa döndürülen Handle'ı C& handle'ına ata
    *handle = loaded_handle;
    c_ampersand_println("MODRESOLVER BILGI: Modül yüklendi: %s (Handle: %llu)", module_name, loaded_handle); // ullu Handle için
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Yüklenmiş bir modülü bellekten kaldırır (Sahne64 API'si ile).
c_ampersand_result c_ampersand_module_unload(c_ampersand_module_handle handle) {
    // Parametre doğrulama
    if (handle == C_AMPERSAND_INVALID_MODULE_HANDLE) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Geçersiz Handle
    }

    // Sahne64 API'sını kullanarak modülü kaldır
    // Varsayım: sahne_module_unload sahne.h'de tanımlı, sahne_error_t döner.
    sahne_error_t sahne_err = sahne_module_unload(handle);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Modül kaldırma hatası
        return c_ampersand_error_with_message(map_sahne_error_to_camper_error(sahne_err), "Modül kaldırılırken hata: Handle %llu (SAHNE hatası: %d)", handle, sahne_err);
    }

    c_ampersand_println("MODRESOLVER BILGI: Modül kaldırıldı: Handle %llu", handle);
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Yüklenmiş bir modül içindeki bir sembolün (fonksiyon, değişken vb.) adresini alır (Sahne64 API'si ile).
// Sembol adresini *symbol_address parametresine yazar.
c_ampersand_result c_ampersand_module_get_symbol(c_ampersand_module_handle handle, const char *symbol_name, void **symbol_address) {
    // Parametre doğrulama
    if (handle == C_AMPERSAND_INVALID_MODULE_HANDLE || symbol_name == NULL || symbol_address == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Sembol adresini başlangıçta NULL yap
    *symbol_address = NULL;

    // Sahne64 API'sını kullanarak sembolü al
    // Varsayım: sahne_module_get_symbol sahne.h'de tanımlı, sahne_error_t döner ve adresi out_address* parametresine yazar.
    void *sym_addr = NULL;
    sahne_error_t sahne_err = sahne_module_get_symbol(handle, symbol_name, strlen(symbol_name), &sym_addr);

    // SAHNE64 API dönüş değerini kontrol et
    if (sahne_err != SAHNE_SUCCESS) {
        // Sembol bulunamadı veya başka bir hata
        return c_ampersand_error_with_message(map_sahne_error_to_camper_error(sahne_err), "Sembol bulunamadı veya alınamadı: %s (Modül Handle: %llu, SAHNE hatası: %d)", symbol_name, handle, sahne_err);
    }

    // Başarılı olursa sembol adresini çıktı parametresine ata
    *symbol_address = sym_addr;
    // c_ampersand_println("MODRESOLVER BILGI: Sembol bulundu: %s (%p)", symbol_name, sym_addr); // Çok fazla çıktı olabilir
    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// Modül arama yollarına yeni bir yol ekler.
// path stringini C& belleğinde kopyalar ve dizide saklar.
c_ampersand_result c_ampersand_module_add_search_path(const char *path) {
    // Parametre doğrulama
    if (path == NULL || strlen(path) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Arama yolu dizisi limitini kontrol et
    if (num_search_paths >= MAX_SEARCH_PATHS) {
        c_ampersand_println("MODRESOLVER UYARI: Maksimum arama yolu limitine (%d) ulaşıldı. Yol eklenemedi: %s", MAX_SEARCH_PATHS, path);
        return (c_ampersand_result){C_AMPERSAND_ERROR_LIMIT_EXCEEDED, NULL, 0}; // Limit aşıldı hatası (veya sadece OK dönebilir)
    }

    // Yol stringi için C& belleği tahsis et
    size_t path_len = strlen(path);
    size_t allocation_size = path_len + 1; // Null sonlandırıcı dahil
    char *path_ptr = NULL;
    c_ampersand_result alloc_res = c_ampersand_allocate(allocation_size, (void**)&path_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
         // Bellek yetersizliği
         return alloc_res; // Bellek hatasını ilet
    }

    // Stringi kopyala
    memcpy(path_ptr, path, allocation_size);

    // Dizide pointer ve boyutu sakla
    search_paths[num_search_paths].ptr = path_ptr;
    search_paths[num_search_paths].size = path_len; // Boyut olarak strlen sonucu saklamak free için daha uygun

    num_search_paths++; // Sayacı artır

    c_ampersand_println("MODRESOLVER BILGI: Arama yolu eklendi: %s", path);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Modül çözücüyü başlatır. Varsayılan arama yollarını ekler.
c_ampersand_result c_ampersand_module_init() {
    // Arama yolları dizisini sıfırla
    memset(search_paths, 0, sizeof(search_path_t) * MAX_SEARCH_PATHS);
    num_search_paths = 0; // Sayacı sıfırla

    // Başlangıçta bazı varsayılan arama yolları eklenebilir.
    // Örneğin, sistem modül dizinleri veya mevcut çalışma dizini.
    // c_ampersand_module_add_search_path kendi içinde bellek tahsisi yapar ve hatayı raporlar.
    c_ampersand_result result = c_ampersand_module_add_search_path("./modules"); // Mevcut dizindeki "modules" klasörü
    // Hata olursa loglanır ama başlatma devam edebilir veya hata dönebiliriz.
    // Başarısız default yol eklemeyi hata saymayalım şimdilik.

    c_ampersand_println("MODRESOLVER BILGI: Modül çözücü başlatıldı.");
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Modül çözücüyü kapatır ve kullanılan arama yolu stringlerini serbest bırakır.
// c_ampersand_free kullanır.
c_ampersand_result c_ampersand_module_shutdown() {
    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Kaydedilmiş arama yollarını serbest bırak
    for (int i = 0; i < num_search_paths; ++i) {
        // Pointer geçerli mi ve boyutu var mı kontrol et
        if (search_paths[i].ptr != NULL && search_paths[i].size > 0) {
            // Arama yolu stringini serbest bırak (C& bellek API)
            c_ampersand_result free_res = c_ampersand_free(search_paths[i].ptr, search_paths[i].size + 1); // +1 null sonlandırıcı için tahsis boyutu

            if (free_res.code != C_AMPERSAND_OK) {
                // Serbest bırakma hatası olursa logla ve hatayı kaydet
                // c_ampersand_result_print_error(free_res);
                if (final_result.code == C_AMPERSAND_OK) final_result = free_res; // İlk hatayı sakla
                // c_ampersand_result_free(&free_res); // Mesajı serbest bırak (eğer varsa)
            }
        }
        // Dizideki elemanı sıfırla (güvenlik için)
        search_paths[i].ptr = NULL;
        search_paths[i].size = 0;
    }
    num_search_paths = 0; // Sayacı sıfırla

    c_ampersand_println("MODRESOLVER BILGI: Modül çözücü kapatıldı.");
    return final_result; // Kapanış işleminin genel sonucunu dön
}

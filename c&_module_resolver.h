#ifndef C_AMPERSAND_MODULE_RESOLVER_H
#define C_AMPERSAND_MODULE_RESOLVER_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (Handle, u64, sahne_error_t, module_* fonksiyonları için)
#include "sahne.h"
// C& G/Ç operasyonları (dosya varlığını kontrol etmek için)
#include "c&_io_operations.h" // c_ampersand_io_open, c_ampersand_file_handle için

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// Yüklenmiş bir modülü temsil eden Handle (Sahne64 Handle'ına karşılık gelir)
typedef u64 c_ampersand_module_handle;

// Geçersiz modül Handle değeri
#define C_AMPERSAND_INVALID_MODULE_HANDLE 0 // Varsayım: SAHNE64'te geçersiz Handle 0


// Bir modülü verilen ada göre arama yollarında bulur ve Sahne64 API'si ile yükler.
// module_name: Yüklenecek modülün adı (örn. "my_module").
// handle: Yüklenen modülün Handle'ının yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_module_load(const char *module_name, c_ampersand_module_handle *handle);

// Yüklenmiş bir modülü bellekten kaldırır (Sahne64 API'si ile).
// handle: Kaldırılacak modülün Handle'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_module_unload(c_ampersand_module_handle handle);

// Yüklenmiş bir modül içindeki bir sembolün (fonksiyon, değişken vb.) adresini alır (Sahne64 API'si ile).
// handle: Sembolün aranacağı modülün Handle'ı.
// symbol_name: Aranacak sembol adı.
// symbol_address: Sembolün adresinin yazılacağı void** işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_module_get_symbol(c_ampersand_module_handle handle, const char *symbol_name, void **symbol_address);

// Modül arama yollarına yeni bir yol ekler.
// path: Eklenecek arama yolu stringi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_module_add_search_path(const char *path);

// Modül çözücüyü ve arama yollarını başlatır.
c_ampersand_result c_ampersand_module_init();

// Modül çözücüyü kapatır ve kullanılan arama yolu kaynaklarını serbest bırakır.
c_ampersand_result c_ampersand_module_shutdown();

// ... diğer modül çözme ile ilgili fonksiyonlar eklenebilir

#endif // C_AMPERSAND_MODULE_RESOLVER_H

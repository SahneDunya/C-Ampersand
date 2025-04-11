#ifndef C_AMPERSAND_MODULE_RESOLVER_H
#define C_AMPERSAND_MODULE_RESOLVER_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stddef.h> // size_t için
#include <stdbool.h>

// Yüklenmiş bir modülü temsil eden opak bir yapı
typedef void *c_ampersand_module_handle;

// Bir modülü belirtir.
typedef char * c_ampersand_module_name;

// Bir modülü verilen ada göre yükler.
c_ampersand_result c_ampersand_module_load(c_ampersand_module_name module_name, c_ampersand_module_handle *handle);

// Yüklenmiş bir modülü bellekten kaldırır.
c_ampersand_result c_ampersand_module_unload(c_ampersand_module_handle handle);

// Yüklenmiş bir modül içindeki bir sembolün (fonksiyon, değişken vb.) adresini alır.
c_ampersand_result c_ampersand_module_get_symbol(c_ampersand_module_handle handle, const char *symbol_name, void **symbol_address);

// Modül arama yollarına yeni bir yol ekler.
c_ampersand_result c_ampersand_module_add_search_path(const char *path);

// Modül çözücüyü başlatır.
c_ampersand_result c_ampersand_module_init();

// Modül çözücüyü kapatır ve kullanılan kaynakları serbest bırakır.
c_ampersand_result c_ampersand_module_shutdown();

// ... diğer modül çözme ile ilgili fonksiyonlar eklenebilir

#endif // C_AMPERSAND_MODULE_RESOLVER_H
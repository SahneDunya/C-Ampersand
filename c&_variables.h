#ifndef C_AMPERSAND_VARIABLES_H
#define C_AMPERSAND_VARIABLES_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// Veri tipleri tanımı için (c_ampersand_type struct)
#include "c&_types.h"
// Değerleri tutmak için (c_ampersand_value struct)
#include "c&_data_types.h"
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"

#include <stdbool.h> // bool için
#include <stddef.h> // size_t için


// Bir değişkeni temsil eden yapı. Heap üzerinde yaşar.
typedef struct c_ampersand_variable {
    char *name; // Değişken adı (heap'te ayrılmış)
    size_t name_allocated_size; // name stringi için ayrılan boyut (free için)
    c_ampersand_type type; // Değişkenin tipi (c_ampersand_type structı)
    c_ampersand_value value; // Değişkenin runtime değeri (c_ampersand_value structı)
    // Bu değişken struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t variable_allocated_size;

    // ... diğer değişken özellikleri eklenebilir (örn. sabit mi, erişim seviyesi)
    struct c_ampersand_variable *next; // Aynı scope içindeki bir sonraki değişken için (basit bağlı liste)
} c_ampersand_variable;

// Bir scope'u (kapsamı) temsil eden yapı. Heap üzerinde yaşar.
typedef struct c_ampersand_scope {
    c_ampersand_variable *variables; // Bu scope'taki değişkenlerin listesinin başı
    struct c_ampersand_scope *parent;   // Üst scope (eğer varsa)
    // Bu scope struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t scope_allocated_size;

    // ... diğer scope bilgileri eklenebilir (örn. adı, derinlik)
} c_ampersand_scope;

// Global scope (kök scope) pointer'ı. Modül dışından erişilebilir.
extern c_ampersand_scope *global_scope;


// Yeni bir değişken oluşturur. Ad stringini ve değişken struct'ını C& belleğinde tahsis eder.
// name: Değişken adı.
// type: Değişkenin tipi.
// Dönüş: Oluşturulan değişken pointer'ı veya bellek/değer oluşturma hatası durumunda NULL.
c_ampersand_variable *c_ampersand_variable_create(const char *name, c_ampersand_type type);

// Bir değişkeni serbest bırakır. Ad stringini, değeri ve değişken struct'ını serbest bırakır.
// c_ampersand_free ve c_ampersand_value_free kullanır. Hataları loglar.
void c_ampersand_variable_free(c_ampersand_variable *var);


// Bir değişkene değer atar (runtime'da). Değerin derin kopyasını yapabilir.
// var: Değerin atanacağı değişken.
// value: Atanacak değer.
// Dönüş: Başarı veya hata (geçersiz argüman, tip uyumsuzluğu, bellek yetersizliği).
c_ampersand_result c_ampersand_variable_set_value(c_ampersand_variable *var, const c_ampersand_value *value);

// Bir değişkenin değerini alır (runtime'da). Değerin derin kopyasını out_value'ya yapar.
// var: Değeri alınacak değişken.
// out_value: Değerin kopyalanacağı c_ampersand_value pointer'ı. Belleği bu fonksiyon içinde tahsis edilebilir.
// Dönüş: Başarı veya hata (geçersiz argüman, bellek yetersizliği). Çağıran out_value'yu c_ampersand_value_free ile serbest bırakmalıdır.
c_ampersand_result c_ampersand_variable_get_value(const c_ampersand_variable *var, c_ampersand_value *out_value);


// Belirtilen scope'ta belirli bir ada sahip bir değişkeni arar (scope hiyerarşisinde yukarı çıkarak).
// scope: Aramanın başlayacağı scope.
// name: Aranacak değişken adı.
// Dönüş: Bulunan değişken pointer'ı veya bulunamazsa NULL.
c_ampersand_variable *c_ampersand_variable_lookup(c_ampersand_scope *scope, const char *name);


// Yeni bir scope oluşturur. Scope struct'ını C& belleğinde tahsis eder.
// parent: Üst scope (NULL global scope için).
// Dönüş: Oluşturulan scope pointer'ı veya bellek yetersizliği durumunda NULL.
c_ampersand_scope *c_ampersand_scope_create(c_ampersand_scope *parent);

// Bir scope'u ve içindeki tüm değişkenleri rekürsif olarak serbest bırakır.
// c_ampersand_free ve c_ampersand_variable_free kullanır. Hataları loglar.
void c_ampersand_scope_free(c_ampersand_scope *scope);

// Belirtilen scope'a bir değişken ekler. Değişkenin aynı scope'ta zaten tanımlı olup olmadığını kontrol eder.
// scope: Değişkenin ekleneceği scope.
// var: Eklenecek değişken (daha önce c_ampersand_variable_create ile oluşturulmuş olmalı).
// Dönüş: Başarı veya hata (geçersiz argüman, değişken zaten tanımlı).
c_ampersand_result c_ampersand_scope_add_variable(c_ampersand_scope *scope, c_ampersand_variable *var);


// Değişken yönetim modülünü başlatır (global scope'u oluşturur).
c_ampersand_result c_ampersand_variables_init();

// Değişken yönetim modülünü kapatır (global scope'u ve içindeki her şeyi serbest bırakır).
c_ampersand_result c_ampersand_variables_shutdown();


// ... diğer değişken ve scope yönetimi ile ilgili fonksiyonlar eklenebilir

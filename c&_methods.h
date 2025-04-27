#ifndef C_AMPERSAND_METHODS_H
#define C_AMPERSAND_METHODS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// C& veri tipleri için (c_ampersand_type_kind)
#include "c&_types.h"
// C& değer yapıları için (c_ampersand_value)
#include "c&_data_types.h"

// size_t için
#include <stddef.h>


// Bir metot implementasyonu için fonksiyon pointer tipi
// Metotlar, 'self' nesnesini, argüman dizisini ve boyutunu, ve sonucun yazılacağı
// bir c_ampersand_value pointer'ını alır. İşlemin sonucunu c_ampersand_result olarak döner.
typedef c_ampersand_result (*c_ampersand_method_func)(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);

// Bir metodu temsil eden yapı
typedef struct c_ampersand_method {
    // Metot adı (heap'te ayrılmış string)
    const char *name;
    size_t name_allocated_size; // name stringi için ayrılan boyut (free için)

    // Bu metodun ait olduğu C& tipi
    c_ampersand_type_kind belongs_to_type;

    // Metot implementasyonunun fonksiyon pointer'ı
    c_ampersand_method_func func_ptr;

    // Aynı tipteki metotlar için bağlı liste (NULL sonlu)
    struct c_ampersand_method *next;

    // Bu struct için ayrılan toplam bellek boyutu (free için)
    size_t allocated_size;
} c_ampersand_method;

// Bir tipin metot tablosunu temsil eden yapı
// Bu yapılar global_method_table listesinde tutulur.
typedef struct c_ampersand_method_table {
    // Bu tablonun ait olduğu C& tipi
    c_ampersand_type_kind type;

    // Bu tipe ait metotların bağlı listesi (NULL sonlu)
    c_ampersand_method *methods;

    // Tüm metot tabloları listesindeki bir sonraki tablo (NULL sonlu)
    struct c_ampersand_method_table *next;

    // Bu struct için ayrılan toplam bellek boyutu (free için)
    size_t allocated_size;
} c_ampersand_method_table;

// Tüm metot tablolarının bağlı listesinin başı.
// Bu global değişken runtime boyunca geçerlidir.
extern c_ampersand_method_table *global_method_table;


// Yeni bir metot yapısı oluşturur ve pointer'ını out_method'a yazar.
// name: Metot adı. Kopyalanır ve metot struct'ına bağlanır.
// type: Metodun ait olduğu C& tipi.
// func_ptr: Metot implementasyonuna işaret eden fonksiyon pointer'ı.
// out_method: Oluşturulan c_ampersand_method pointer'ının yazılacağı void** pointer.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek hatalarını raporlar.
c_ampersand_result c_ampersand_method_create(const char *name, c_ampersand_type_kind type, c_ampersand_method_func func_ptr, c_ampersand_method **out_method);

// Bir metot yapısını ve içerdiği string belleğini serbest bırakır.
// method: Serbest bırakılacak metot yapısının pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek hatalarını raporlar.
c_ampersand_result c_ampersand_method_free(c_ampersand_method *method);

// Belirli bir tipe bir metot ekler.
// type: Metodun ekleneceği C& tipi.
// method: Eklenecek metot yapısının pointer'ı (daha önce create ile oluşturulmuş olmalı).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek hatalarını (yeni tablo oluşturma) veya geçersiz argümanları raporlar.
c_ampersand_result c_ampersand_method_register(c_ampersand_type_kind type, c_ampersand_method *method);

// Bir nesnenin tipi ve metot adına göre metodu arar.
// object_type: Metodu aranacak nesnenin C& tipi.
// method_name: Aranacak metot adı.
// Dönüş: Bulunan c_ampersand_method yapısının pointer'ı veya bulunamazsa NULL.
// NOT: Bu fonksiyon bellek ayırmaz veya serbest bırakmaz.
c_ampersand_method *c_ampersand_method_lookup(c_ampersand_type_kind object_type, const char *method_name);

// Bir nesne üzerinde bir metodu çağırır.
// object: Metodun çağrılacağı c_ampersand_value pointer'ı (self).
// method_name: Çağrılacak metot adı.
// args: Metoda iletilen argümanların c_ampersand_value dizisi.
// num_args: Argüman sayısı.
// result: Metot sonucunun yazılacağı c_ampersand_value pointer'ı.
// Dönüş: Metot implementasyonunun döndürdüğü c_ampersand_result veya çağırma sırasında oluşan hata.
c_ampersand_result c_ampersand_method_call(c_ampersand_value *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);

// Metot sistemini (global tabloyu) başlatır.
c_ampersand_result c_ampersand_methods_init();

// Metot sistemini (global tabloyu ve tüm metotları) kapatır ve kullanılan belleği serbest bırakır.
// c_ampersand_method_free ve c_ampersand_free kullanır.
c_ampersand_result c_ampersand_methods_shutdown();


// Örnek Metot Implementasyonlarının Bildirimleri (c&_methods.c'de tanımlanacak)
 string.length()
extern c_ampersand_result string_length(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);
 int.to_string(), long.to_string()
extern c_ampersand_result int_to_string(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);


#endif // C_AMPERSAND_METHODS_H

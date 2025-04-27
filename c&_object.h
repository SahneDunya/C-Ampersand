#ifndef C_AMPERSAND_OBJECT_H
#define C_AMPERSAND_OBJECT_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// C& veri tipleri için (c_ampersand_type, c_ampersand_type_kind)
#include "c&_types.h"
// C& değer yapıları için (c_ampersand_value - union içeren, string_val ptr/size)
#include "c&_data_types.h"
// Metotlar için (c_ampersand_method_call)
#include "c&_methods.h"

// size_t için
#include <stddef.h>


// Bir nesneyi temsil eden yapı (heap üzerinde yaşar)
typedef struct c_ampersand_object {
    // Nesnenin ait olduğu tip
    c_ampersand_type type; // Varsayım: c_ampersand_type tip bilgisini tutar

    // Nesnenin verisine işaretçi (tipin alanlarının depolandığı buffer)
    // Primitif tipler için bu buffer o primitif değeri doğrudan tutar.
    // String gibi kompleks tipler için, bu buffer genellikle o kompleks tipin
    // heap pointer'ını tutar (örn. string için char* pointer'ı).
    void *data;

    // 'data' buffer'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t data_allocated_size;

    // Bu c_ampersand_object struct'ının kendisi için c_ampersand_allocate ile ayrılan boyut.
    size_t allocated_size;

    // GC yönetimi için bayraklar veya alanlar eklenebilir (örn. marked)
    // bool is_gc_managed;
    // bool marked;
} c_ampersand_object;


// Yeni bir nesne oluşturur ve pointer'ını out_object'a yazar.
// Nesne struct'ı ve veri buffer'ı için c_ampersand_allocate kullanır.
// type: Oluşturulacak nesnenin tipi.
// out_object: Oluşturulan c_ampersand_object pointer'ının yazılacağı void** pointer.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek hatalarını raporlar.
c_ampersand_result c_ampersand_object_create(c_ampersand_type type, c_ampersand_object **out_object);

// Bir nesneyi ve içerdiği veri buffer'ını serbest bırakır.
// String gibi veri buffer'ının içinde ayrıca heap belleği tutan tipler için
// o içsel bellek de serbest bırakılır. c_ampersand_free kullanır.
// object: Serbest bırakılacak nesne yapısının pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek hatalarını raporlar.
c_ampersand_result c_ampersand_object_free(c_ampersand_object *object);

// Bir nesnenin belirtilen alanının değerini alır ve out_value'a kopyalar.
// Şu anki taslakta alan erişimi kısıtlıdır (sadece placeholder).
// object: Alanı alınacak nesne.
// field_name: Alınacak alan adı.
// out_value: Alan değerinin kopyalanacağı c_ampersand_value pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Alan bulunamaması veya tip hatası gibi durumları raporlar.
c_ampersand_result c_ampersand_object_get_field(const c_ampersand_object *object, const char *field_name, c_ampersand_value *out_value);

// Bir nesnenin belirtilen alanının değerini ayarlar.
// Şu anki taslakta alan erişimi kısıtlıdır (sadece placeholder).
// object: Alanı ayarlanacak nesne.
// field_name: Ayarlanacak alan adı.
// value: Alanın yeni değeri (c_ampersand_value pointer'ı).
// Dönüş: Başarı veya hata belirten c_ampersand_result. Alan bulunamaması veya tip hatası gibi durumları raporlar.
c_ampersand_result c_ampersand_object_set_field(c_ampersand_object *object, const char *field_name, const c_ampersand_value *value);

// Bir nesne üzerinde belirtilen metodu çağırır.
// object: Metodun çağrılacağı nesne (self argümanı olarak kullanılır).
// method_name: Çağrılacak metot adı.
// args: Metoda iletilen argümanların c_ampersand_value dizisi.
// num_args: Argüman sayısı.
// result: Metot sonucunun yazılacağı c_ampersand_value pointer'ı.
// Dönüş: Metot çağrısının sonucu (c_ampersand_result).
c_ampersand_result c_ampersand_object_call_method(c_ampersand_object *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result);


#endif // C_AMPERSAND_OBJECT_H

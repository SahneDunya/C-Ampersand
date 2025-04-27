#include "c&_object.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println için
#include "c&_stdlib.h"
// C& veri tipleri (c_ampersand_type, c_ampersand_type_kind) ve tip bilgisi fonksiyonları için (örn. size)
#include "c&_types.h" // Varsayım: c_ampersand_type_get_allocation_size burada bildirildi/tanımlandı
// C& değer yapıları (c_ampersand_value - union içeren, string_val ptr/size) için
#include "c&_data_types.h"
// Metotlar için (c_ampersand_method_call)
#include "c&_methods.h"
// C& bellek yönetimi fonksiyonları için (Sahne64 API sarmalayıcıları)
#include "c&_memory.h"

// sahne64_api.h kaldırıldı
// #include "sahne64_api.h" // Bellek yönetimi için

// stdlib.h (malloc, free) ve stdio.h (fprintf) kaldırıldı
// #include <stdlib.h>
// #include <stdio.h>

#include <string.h> // strlen, strcmp, memcpy için
#include <stdint.h> // int32_t, uint32_t vb. için (c&_types.h de sağlayabilir)
#include <stdbool.h> // bool için


// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
// static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);

// Varsayım: c_ampersand_type_get_allocation_size fonksiyonu c&_types.c'de tanımlı
// size_t c_ampersand_type_get_allocation_size(c_ampersand_type type);

// --- Nesne Yönetimi ---

// Yeni bir nesne oluşturur ve pointer'ını out_object'a yazar.
// Nesne struct'ı ve veri buffer'ı için c_ampersand_allocate kullanır.
c_ampersand_result c_ampersand_object_create(c_ampersand_type type, c_ampersand_object **out_object) {
    // Parametre doğrulama
    if (out_object == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // out_object pointer'ını başlangıçta NULL yap
    *out_object = NULL;

    // Nesne struct'ı için bellek tahsis et
    c_ampersand_object *object = NULL;
    size_t object_struct_size = sizeof(c_ampersand_object);
    c_ampersand_result alloc_res = c_ampersand_allocate(object_struct_size, (void**)&object);
    if (alloc_res.code != C_AMPERSAND_OK) {
        return alloc_res; // Bellek hatasını ilet
    }

    // Tahsis edilen nesne belleğini sıfırla
    memset(object, 0, object_struct_size);

    // Nesne tipi ve tahsis boyutunu ayarla
    object->type = type; // c_ampersand_type structı kopyalanır
    object->allocated_size = object_struct_size; // Nesne struct'ının boyutunu kaydet

    // Nesnenin veri buffer'ı için gerekli boyutu belirle
    size_t data_size = c_ampersand_type_get_allocation_size(type); // c&_types.c'den gelen fonksiyon varsayımı

    // Veri buffer'ı için bellek ayır (boyut > 0 ise)
    void *data_buffer = NULL;
    if (data_size > 0) {
        alloc_res = c_ampersand_allocate(data_size, &data_buffer);
        if (alloc_res.code != C_AMPERSAND_OK) {
            // Veri buffer'ı tahsis hatası, nesne struct'ını serbest bırak
            c_ampersand_free(object, object_struct_size); // Hatanın dönüş değerini kontrol etmiyoruz (kritik hata olabilir)
            return alloc_res; // Bellek hatasını ilet
        }
        // Veri buffer'ını sıfırla
        memset(data_buffer, 0, data_size);
    }

    // Nesneye veri pointer'ı ve boyutunu ata
    object->data = data_buffer;
    object->data_allocated_size = data_size; // Veri buffer'ının boyutunu kaydet

    // String gibi bazı tipler için ek başlangıç değerleri gerekebilir
    if (type.kind == TYPE_STRING) {
        // object->data artık stringin ptr/size structını tutuyor.
        // Bu struct'ın pointer alanını NULL ve size alanını 0 yapmalıyız.
        // data_buffer'ı ilgili struct tipine cast edelim.
        // Varsayım: c_ampersand_value'daki string_val structı aynı layout'a sahip.
        // object->data'nın c_ampersand_value::data::string_val structını tuttuğu varsayılırsa:
        // *((struct { char* ptr; size_t size; }*)object->data) = (struct { char* ptr; size_t size; }){ NULL, 0 };
        // Veya daha genel:
        memset(object->data, 0, data_size); // zaten yapıldı ama tekrar belirtelim.
    }

    *out_object = object; // Oluşturulan nesne pointer'ını çıktıya yaz
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir nesneyi ve içerdiği veri buffer'ını serbest bırakır.
// String gibi veri buffer'ının içinde ayrıca heap belleği tutan tipler için
// o içsel bellek de serbest bırakılır. c_ampersand_free kullanır.
c_ampersand_result c_ampersand_object_free(c_ampersand_object *object) {
    // NULL nesne kontrolü
    if (object == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL pointer'ı serbest bırakmak güvenlidir
    }

    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Veri buffer'ı varsa serbest bırak
    if (object->data != NULL && object->data_allocated_size > 0) {

        // Eğer string gibi veri buffer'ının içinde ayrıca heap belleği tutan bir tipse,
        // önce o içsel belleği serbest bırak
        if (object->type.kind == TYPE_STRING) {
             // object->data, c_ampersand_value::data::string_val structını tutuyor varsayımıyla
             struct { char* ptr; size_t size; } *string_data_ptr = (struct { char* ptr; size_t size; }*)object->data;

             if (string_data_ptr->ptr != NULL && string_data_ptr->size > 0) {
                 // String içeriğini serbest bırak (c_ampersand_free boyuta ihtiyaç duyar)
                 // String içeriğinin boyutu string_data_ptr->size alanında saklı.
                 c_ampersand_result free_content_res = c_ampersand_free(string_data_ptr->ptr, string_data_ptr->size + 1); // +1 null sonlandırıcı için
                 if (free_content_res.code != C_AMPERSAND_OK) {
                     // Hata olursa kaydet
                     if (final_result.code == C_AMPERSAND_OK) final_result = free_content_res;
                 }
                 string_data_ptr->ptr = NULL; // Pointer'ı NULL yap
                 string_data_ptr->size = 0;
             }
        }
         // ... diğer kompleks tipler için de benzer içsel serbest bırakma mantığı ...

        // Veri buffer'ının kendisini serbest bırak
        c_ampersand_result free_data_res = c_ampersand_free(object->data, object->data_allocated_size);
         if (free_data_res.code != C_AMPERSAND_OK) {
             // Hata olursa kaydet
             if (final_result.code == C_AMPERSAND_OK) final_result = free_data_res;
         }
         object->data = NULL; // Pointer'ı NULL yap
         object->data_allocated_size = 0;
    }

    // Nesne struct'ının kendisini serbest bırak
    // allocated_size alanı create sırasında ayarlanmıştı.
    if (object->allocated_size > 0) {
        c_ampersand_result free_object_res = c_ampersand_free(object, object->allocated_size);
        if (free_object_res.code != C_AMPERSAND_OK) {
            // Hata olursa kaydet
            if (final_result.code == C_AMPERSAND_OK) final_result = free_object_res;
        }
    } else {
        // Bu bir programlama hatasıdır, allocated_size 0 olmamalıydı.
        // Loglama yapılabilir. Hata döndürmek bu noktada zor olabilir.
        c_ampersand_println("OBJECT HATA: Free edilmek istenen nesne allocated_size 0. Bellek bozulmuş olabilir.");
        // Belki de final_result'a bir hata atamalıyız.
        if (final_result.code == C_AMPERSAND_OK) final_result = (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Kaba bir hata
    }


    // Nesne pointer'ını NULL yap (caller'ın dangle pointer'ı kullanmasını engellemez)
    // Ama nesne serbest bırakıldıysa onu tekrar kullanmamalıyız.

    return final_result; // Genel serbest bırakma sonucunu dön
}

// Bir nesnenin belirtilen alanının değerini alır ve out_value'a kopyalar.
// Alan erişimi için tip bilgisi kullanılır (şu anki taslakta eksik).
// out_value, alan değerinin bir kopyasını (string için pointer/size) tutacaktır.
c_ampersand_result c_ampersand_object_get_field(const c_ampersand_object *object, const char *field_name, c_ampersand_value *out_value) {
    // Parametre doğrulama
    if (object == NULL || field_name == NULL || out_value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // out_value'yu başlangıçta sıfırla
    memset(out_value, 0, sizeof(c_ampersand_value));
    // out_value->type.kind = TYPE_UNKNOWN; // Veya daha spesifik bir sıfırlama

    // Gerçek bir implementasyonda:
    // 1. object->type tanımından "field_name" alanını ara.
    // 2. Alanın tipini (field_type), ofsetini (field_offset) ve boyutunu (field_size) al.
    // 3. Alan verisinin başlangıç adresini hesapla: (char*)object->data + field_offset.
    // 4. Alanın tipine göre değeri kopyala/eriş.

    // --- Basit Örnek Alan Erişimi (Placeholder) ---
    // SADECE TEK BİR "value" ALANI VARMIŞ GİBİ DAVRANIR.
    // BU KISIM GERÇEK TIP VE ALAN YÖNETİMİ İLE DEĞİŞTİRİLMELİDİR.

    if (strcmp(field_name, "value") == 0) { // Örnek bir alan adı
        // out_value'nun tipini nesnenin tipiyle aynı yap (basit alan varsayımıyla)
        out_value->type = object->type; // c_ampersand_type structı kopyalanır

        if (object->type.kind == TYPE_STRING) {
             // object->data, c_ampersand_value::data::string_val structını tutuyor varsayımıyla
             struct { char* ptr; size_t size; } *string_data_ptr = (struct { char* ptr; size_t size; }*)object->data;

            // String value'yu kopyala (pointer ve size kopyalama, içerik kopyalama DEĞİL)
            out_value->data.string_val.ptr = string_data_ptr->ptr;
             out_value->data.string_val.size = string_data_ptr->size;
             // Dikkat: Eğer string value semantiği derin kopyalama gerektiriyorsa,
             // burada string içeriği c_ampersand_allocate + memcpy ile kopyalanmalıydı.
             // Varsayım: Value kopyalama string içeriğini kopyalamaz, sadece referansı (ptr/size) kopyalar.

        } else {
             // Diğer primitif tipler için veri buffer'ından değeri kopyala
             // Boyut c_ampersand_type_get_allocation_size(object->type) ile alınmalı
             size_t data_size = c_ampersand_type_get_allocation_size(object->type); // Tip boyutunu al
             if (data_size > 0 && object->data != NULL) {
                // out_value->data bir union. object->data'daki veriyi bu union'ın ilgili alanına kopyalamalıyız.
                // Bu, alan tipini bilmeyi gerektirir (ki burada varsayılan 'value' alanı nesnenin tipiyle aynı varsayılıyor).
                // memcpy(&out_value->data, object->data, data_size); // value->data'ya kopyala (union'a)
                // Bu tehlikelidir, union'ın tümünü kopyalamak yerine ilgili alana kopyalanmalı.
                // Örneğin: if (object->type.kind == TYPE_INT32) out_value->data.i32_val = *(int32_t*)object->data;
                // Basitlik adına, object->data'nın ilgili tipin verisini doğrudan tuttuğunu varsayalım.
                // Ve out_value->data'nın da aynı layout'a sahip olduğunu varsayalım (ki value union içerir).
                // Doğrusu: Alan tipine göre cast edip atama yapmak veya alan boyutunda memcpy yapmak.
                // memcpy(&(out_value->data), object->data, data_size); // Kaba kopyalama
                // Daha doğru: Tipin kind'ına göre union alanına atama yap.
                switch(object->type.kind) {
                     case TYPE_INT32: out_value->data.i32_val = *(int32_t*)object->data; break;
                     case TYPE_INT64: out_value->data.i64_val = *(int64_t*)object->data; break;
                     case TYPE_FLOAT32: out_value->data.f32_val = *(float*)object->data; break;
                     case TYPE_FLOAT64: out_value->data.f64_val = *(double*)object->data; break;
                     case TYPE_BOOL: out_value->data.bool_val = *(bool*)object->data; break;
                     case TYPE_CHAR: out_value->data.char_val = *(char*)object->data; break;
                     case TYPE_POINTER: out_value->data.ptr_val = *(void**)object->data; break;
                     // ... diğer primitif tipler
                     default:
                         // Bilinmeyen veya kompleks tip (string hariç)
                         // Eğer data_size > 0 ise, bu bir struct veya array gibi olabilir.
                         // out_value'nun bu tür veriyi nasıl kopyalayacağı c_ampersand_value tanımına bağlıdır.
                         // Şu anki c_ampersand_value'da bu tür veriler için özel alan yok.
                         // Sadece pointer kopyalama yapabiliriz, ama bu tehlikeli olabilir.
                         c_ampersand_println("OBJECT UYARI: Alan değeri alınırken kompleks/bilinmeyen tip (%d). Pointer kopyalanıyor olabilir.", object->type.kind);
                         out_value->data.ptr_val = object->data; // Sadece pointerı kopyala (muhtemelen yanlış)
                         break;
                }

             } else {
                 // Data size 0 veya data NULL. Value 0/NULL olarak kalır (memset ile sıfırlandı).
             }
        }
        return C_AMPERSAND_OK_RESULT; // Başarı
    }

    // Alan "value" değilse veya alan erişim mantığı implemente edilmediyse
    // Varsayım: C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION c&_stdlib.h'de tanımlı
    return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Alan bulunamadı veya erişilemiyor: %s (Tip: %d)", field_name, object->type.kind);
}

// Bir nesnenin belirtilen alanının değerini ayarlar.
// Alan erişimi için tip bilgisi kullanılır (şu anki taslakta eksik).
// value c_ampersand_value'daki değer nesneye kopyalanır/ayarlanır.
c_ampersand_result c_ampersand_object_set_field(c_ampersand_object *object, const char *field_name, const c_ampersand_value *value) {
    // Parametre doğrulama
    if (object == NULL || field_name == NULL || value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Not: Tip uyumu kontrolü alana özel olmalı, nesnenin tipiyle değerin ana tipi değil.
    // Örneğin int alanına int değer atanabilir. Ama int[] nesnesine int[] değer atanırken alanlar eşleşmeli.
    // Basitlik adına, şu anki 'value' alanı için tiplerin tam uyuşması varsayılıyor.
    // if (object->type.kind != value->type.kind) { // Bu kontrol her alan için ayrı yapılmalı
    //     return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    // }


    // Gerçek bir implementasyonda:
    // 1. object->type tanımından "field_name" alanını ara.
    // 2. Alanın tipini (field_type), ofsetini (field_offset) ve boyutunu (field_size) al.
    // 3. value'nun tipi field_type ile uyumlu mu kontrol et.
    // 4. Alan verisinin başlangıç adresini hesapla: (char*)object->data + field_offset.
    // 5. Alanın tipine göre değeri kopyala/ayarla.

    // --- Basit Örnek Alan Ayarlama (Placeholder) ---
    // SADECE TEK BİR "value" ALANI VARMIŞ GİBİ DAVRANIR.
    // BU KISIM GERÇEK TIP VE ALAN YÖNETİMİ İLE DEĞİŞTİRİLMELİDİR.

    if (strcmp(field_name, "value") == 0) { // Örnek bir alan adı
        // Alanın tipi nesnenin tipiyle aynı varsayılıyor ve value'nun tipi de bu tiple aynı varsayılıyor.
        if (object->type.kind != value->type.kind) { // Tip uyumu kontrolü
             return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, c_ampersand_error_with_message(C_AMPERSAND_ERROR_TYPE_MISMATCH, "Tip uyuşmazlığı: Alan tipi (%d) değer tipiyle (%d) eşleşmiyor.", object->type.kind, value->type.kind).message, 0};
        }

        if (object->type.kind == TYPE_STRING) {
             // object->data, c_ampersand_value::data::string_val structını tutuyor varsayımıyla
             struct { char* ptr; size_t size; } *object_string_data_ptr = (struct { char* ptr; size_t size; }*)object->data;

            // Eski string içeriğini serbest bırak (varsa)
            if (object_string_data_ptr->ptr != NULL && object_string_data_ptr->size > 0) {
                c_ampersand_free(object_string_data_ptr->ptr, object_string_data_ptr->size + 1); // +1 null sonlandırıcı için tahsis boyutu
                // Free hatası burada raporlanmalı ama devam ediyoruz (kritik hata olabilir).
            }

            // Yeni string içeriği için bellek tahsis et
            // value->data.string_val value'daki stringin ptr/size structı varsayımıyla
            size_t new_string_len = value->data.string_val.size;
            size_t new_string_alloc_size = new_string_len + 1; // Null sonlandırıcı dahil
            char *new_string_ptr = NULL;
            c_ampersand_result alloc_res = c_ampersand_allocate(new_string_alloc_size, (void**)&new_string_ptr);

            if (alloc_res.code != C_AMPERSAND_OK) {
                // Bellek tahsis hatası
                // object'in string verisi artık boş/NULL olarak kalır.
                object_string_data_ptr->ptr = NULL;
                object_string_data_ptr->size = 0;
                return alloc_res; // OOM hatasını ilet
            }

            // Yeni string içeriğini kopyala
            memcpy(new_string_ptr, value->data.string_val.ptr, new_string_alloc_size); // Source size with null terminator

            // Nesnedeki string pointer ve boyutunu güncelle
            object_string_data_ptr->ptr = new_string_ptr;
            object_string_data_ptr->size = new_string_len;

        } else {
             // Diğer primitif tipler için değeri doğrudan veri buffer'ına kopyala
             // Boyut c_ampersand_type_get_allocation_size(object->type) ile alınmalı
             size_t data_size = c_ampersand_type_get_allocation_size(object->type); // Tip boyutunu al
             if (data_size > 0 && object->data != NULL) {
                // value->data bir union. Bu union'daki veriyi object->data'ya kopyalamalıyız.
                // Bu, alan tipini bilmeyi gerektirir ve memcpy kullanımı union layoutuna bağlıdır.
                // Doğrusu: value'daki ilgili alan tipine göre cast edip object->data'ya atama yap.
                // Veya field_size kadar memcpy(&((char*)object->data)[field_offset], &value->data, field_size);
                // Şu anki basit 'value' alanı varsayımıyla:
                memcpy(object->data, &(value->data), data_size); // value'nun data union'ını object->data'ya kopyala (riskli!)
             } else {
                 // Data size 0 veya data NULL. Ayarlama yapılamaz.
                 c_ampersand_println("OBJECT UYARI: Alan değeri ayarlanırken data size 0 veya data NULL.");
                 // Belki hata dönülmeli?
             }
        }
        return C_AMPERSAND_OK_RESULT; // Başarı
    }

    // Alan "value" değilse veya alan erişim mantığı implemente edilmediyse
    // Varsayım: C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION c&_stdlib.h'de tanımlı
    return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Alan bulunamadı veya ayarlanamıyor: %s (Tip: %d)", field_name, object->type.kind);
}

// Bir nesne üzerinde belirtilen metotu çağırır.
// Nesneyi (object) c_ampersand_value* olarak c_ampersand_method_call'a geçirir.
// Varsayım: c_ampersand_method_call nesne pointerını self argümanı olarak doğru şekilde işler.
c_ampersand_result c_ampersand_object_call_method(c_ampersand_object *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    // Parametre doğrulama
    if (object == NULL || method_name == NULL || result == NULL) { // args NULL olabilir
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Nesneyi (object) c_ampersand_value* olarak c_ampersand_method_call'a geçir
    // Bu cast, c_ampersand_value'nun c_ampersand_object ile başlangıç alanlarının uyumlu
    // olduğunu veya c_ampersand_method_call'ın bu cast'i işleyebildiğini varsayar.
    return c_ampersand_method_call((c_ampersand_value *)object, method_name, args, num_args, result);
}

// --- Yardımcı Fonksiyon (c&_types.c içinde olmalıdır) ---
// Bu fonksiyon, nesnenin 'data' buffer'ı için tahsis edilecek boyutu döndürür.
// String gibi bazı tipler için bu, sadece bir pointer veya struct boyutu olabilir.
// Diğer tipler için, alanların toplam boyutu olabilir.
// Bu implementasyon, c&_types.c'ye taşınmalı ve orada tüm tipler için doğru boyutları hesaplamalıdır.
size_t c_ampersand_type_get_allocation_size(c_ampersand_type type) {
    switch (type.kind) {
        case TYPE_VOID: return 0; // void için data yok
        case TYPE_BOOL: return sizeof(bool);
        case TYPE_CHAR: return sizeof(char);
        case TYPE_INT8: return sizeof(int8_t);
        case TYPE_INT16: return sizeof(int16_t);
        case TYPE_INT32: return sizeof(int32_t);
        case TYPE_INT64: return sizeof(int64_t);
        case TYPE_UINT8: return sizeof(uint8_t);
        case TYPE_UINT16: return sizeof(uint16_t);
        case TYPE_UINT32: return sizeof(uint32_t);
        case TYPE_UINT64: return sizeof(uint64_t);
        case TYPE_FLOAT32: return sizeof(float);
        case TYPE_FLOAT64: return sizeof(double);
        case TYPE_STRING: return sizeof(struct { char* ptr; size_t size; }); // String verisi ptr/size structı tutar
        case TYPE_POINTER: return sizeof(void *); // Genel pointer tipi
        case TYPE_ARRAY: // Array ve Struct gibi kompleks tipler için boyut hesaplaması daha karmaşıktır.
        case TYPE_STRUCT:
        case TYPE_OBJECT: // Belki de TYPE_OBJECT'in kendisi bir pointer tutar, veri buffer'ı olmaz.
        case TYPE_FUNCTION: // Fonksiyonlar veri tutmayabilir veya bir pointer tutabilir.
        case TYPE_METHOD: // Metotlar veri tutmayabilir veya bir pointer tutabilir.
        case TYPE_MODULE: // Modüller veri tutmayabilir veya bir pointer/handle tutabilir.
        case TYPE_RESOURCE: // Kaynaklar Handle (u64) tutar.
             // Gerçek implementasyonda, tip tanımından alan/eleman boyutları ve sayısı alınarak toplam boyut hesaplanır.
             c_ampersand_println("OBJECT UYARI: c_ampersand_type_get_allocation_size için kompleks/bilinmeyen tip (%d). Boyut 0 varsayıldı.", type.kind);
             return 0; // Bilinmeyen veya karmaşık tipler için şimdilik 0 dönelim (hatalı olabilir)
        default:
             c_ampersand_println("OBJECT UYARI: c_ampersand_type_get_allocation_size için varsayılan durum, bilinmeyen tip (%d). Boyut 0 varsayıldı.", type.kind);
             return 0;
    }
}

#include "c&_data_types.h"
#include "c&_types.h"      // Varsayım: c_ampersand_type tanımı ve TYPE_* enum'ı
#include "c&_stdlib.h"     // Varsayım: c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT tanımları
#include "sahne.h"         // SAHNE64 C API başlığı (Bellek yönetimi için kullanacağız)

#include <string.h>        // strcmp ve strlen için

// --- c&_types.h içinde tanımlanan harici değişkenlerin tanımı (varsayım: bu sabitler burada tanımlanıyor) ---

const c_ampersand_type c_ampersand_type_void =    {TYPE_VOID};
const c_ampersand_type c_ampersand_type_bool =    {TYPE_BOOL};
const c_ampersand_type c_ampersand_type_int8 =    {TYPE_INT8};
const c_ampersand_type c_ampersand_type_int16 =   {TYPE_INT16};
const c_ampersand_type c_ampersand_type_int32 =   {TYPE_INT32};
const c_ampersand_type c_ampersand_type_int64 =   {TYPE_INT64};
const c_ampersand_type c_ampersand_type_uint8 =   {TYPE_UINT8};
const c_ampersand_type c_ampersand_type_uint16 =  {TYPE_UINT16};
const c_ampersand_type c_ampersand_type_uint32 =  {TYPE_UINT32};
const c_ampersand_type c_ampersand_type_uint64 =  {TYPE_UINT64};
const c_ampersand_type c_ampersand_type_float32 = {TYPE_FLOAT32};
const c_ampersand_type c_ampersand_type_float64 = {TYPE_FLOAT64};
const c_ampersand_type c_ampersand_type_string =  {TYPE_STRING};
const c_ampersand_type c_ampersand_type_pointer = {TYPE_POINTER};
// Varsayım: TYPE_* enum'ı da c&_types.h'de tanımlı

// Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de const olarak tanımlı
// const c_ampersand_result C_AMPERSAND_OK_RESULT = { SAHNE_SUCCESS };

// Varsayım: Hata kodları c&_stdlib.h'de tanımlı
// const int C_AMPERSAND_ERROR_INVALID_ARGUMENT = 3; // Örnek
// const int C_AMPERSAND_ERROR_OUT_OF_MEMORY = 1; // Örnek
// const int C_AMPERSAND_ERROR_TYPE_MISMATCH = ?; // Örnek
// Varsayım: SAHNE_ERROR_to_C_AMPERSAND gibi bir eşleme fonksiyonu/makro da c&_stdlib.h'de tanımlı

const char *c_ampersand_type_to_string(c_ampersand_type type) {
    // Varsayım: type.kind geçerli bir TYPE_* değeri
    switch (type.kind) {
        case TYPE_VOID:    return "void";
        case TYPE_BOOL:    return "bool";
        case TYPE_INT8:    return "int8";
        case TYPE_INT16:   return "int16";
        case TYPE_INT32:   return "int32";
        case TYPE_INT64:   return "int64";
        case TYPE_UINT8:   return "uint8";
        case TYPE_UINT16:  return "uint16";
        case TYPE_UINT32:  return "uint32";
        case TYPE_UINT64:  return "uint64";
        case TYPE_FLOAT32: return "float32";
        case TYPE_FLOAT64: return "float64";
        case TYPE_STRING:  return "string";
        case TYPE_POINTER: return "pointer";
        // Diğer tipler buraya eklenmeli (örn. handle tipi gibi Sahne64'e özel tipler)
        default:           return "unknown"; // Bilinmeyen veya hata durumu
    }
}

// --- c&_data_types.h içinde tanımlanan fonksiyonların implementasyonu ---

c_ampersand_result c_ampersand_value_create(c_ampersand_type type, c_ampersand_value *out_value) {
    if (out_value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    out_value->type = type;
    // Tüm birliğin (union) içeriğini başlangıç değeriyle sıfırlayalım (güvenlik için)
    // Bu, string ve pointer alanlarını da NULL/0 yapar.
    memset(&out_value->data, 0, sizeof(out_value->data));

    // Bazı tipler için özel başlangıç değerleri atanabilir (memset 0 ile aynı etkiyi verir string ve pointer için)
    // if (type.kind == TYPE_STRING) {
    //     out_value->data.string_val.ptr = NULL; // Başlangıçta boş string
    //     out_value->data.string_val.size = 0;
    // } else if (type.kind == TYPE_POINTER) {
    //     out_value->data.pointer_val = NULL;
    // }
    // ... diğer Sahne64 özel tipleri için (örn. Handle = 0)

    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_value_free(c_ampersand_value *value) {
    if (value == NULL) {
        return;
    }
    // Heap'te ayrılmış belleği serbest bırak
    if (value->type.kind == TYPE_STRING && value->data.string_val.ptr != NULL) {
        // Sahne64 bellek serbest bırakma sistem çağrısını kullan
        // Serbest bırakma hataları genellikle göz ardı edilir ama loglanması iyi olabilir.
        // sahne_error_t sahne_err =
        sahne_mem_release(value->data.string_val.ptr, value->data.string_val.size);
        value->data.string_val.ptr = NULL;
        value->data.string_val.size = 0;
    }
    // Diğer serbest bırakılması gereken tipler için buraya eklemeler yapılabilir (örn. Sahne64 Handle'ları?)
    // Sahne64 Handle'ları sadece bir u64 olduğu için free etmeye gerek yoktur, ancak
    // Sahne64 API'si handle'ları resource_release ile kapatmayı gerektiriyorsa, o çağrı burada yapılmalıdır.
    // Örneğin:
    // if (value->type.kind == TYPE_SAHNE64_HANDLE && value->data.handle_val != 0) {
    //    sahne_resource_release((sahne_handle_t)value->data.handle_val); // handle_val u64 olmalı
    //    value->data.handle_val = 0;
    // }

    // Not: Primitive tipler için free gerekmez. Pointer tipler genellikle yönetilen belleğe işaret etmez.
}

c_ampersand_result c_ampersand_value_set_bool(c_ampersand_value *value, bool val) {
    if (value == NULL || value->type.kind != TYPE_BOOL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Mevcut string gibi heap belleğini serbest bırak (eğer varsa ve farklı bir türe ayarlanıyorsa)
    // Ancak set fonksiyonları genellikle değerin ZATEN doğru türde olduğunu varsayar,
    // aksi durumda ayrı bir 'convert' fonksiyonu daha temizdir.
    // Şimdilik sadece aynı türde set yapıldığını varsayalım.
    value->data.bool_val = val;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_int(c_ampersand_value *value, int64_t val) {
    if (value == NULL || (value->type.kind < TYPE_INT8 || value->type.kind > TYPE_INT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Burada tür kontrolü ve dönüşüm/taşma kontrolü yapılabilir (örneğin, int32_t'ye sığmayan bir değer verilirse hata).
    // Basitlik adına, gelen int64_t'yi hedef tipin union alanına kopyalıyoruz.
    // Hedef tip int8 ise, sadece alt 8 bit kopyalanır (taşma oluşur). Gerçek implementasyonda bu engellenmeli.
    switch (value->type.kind) {
        case TYPE_INT8: value->data.i8_val = (int8_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_INT16: value->data.i16_val = (int16_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_INT32: value->data.i32_val = (int32_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_INT64: value->data.i64_val = val; break;
        default: return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Zaten yukarıda kontrol edildi, ekstra güvenlik
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_uint(c_ampersand_value *value, uint64_t val) {
    if (value == NULL || (value->type.kind < TYPE_UINT8 || value->type.kind > TYPE_UINT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
     // Burada tür kontrolü ve dönüşüm/taşma kontrolü yapılmalı
    switch (value->type.kind) {
        case TYPE_UINT8: value->data.u8_val = (uint8_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_UINT16: value->data.u16_val = (uint16_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_UINT32: value->data.u32_val = (uint32_t)val; break; // Taşma kontrolü yapılmalı
        case TYPE_UINT64: value->data.u64_val = val; break;
        default: return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Ekstra güvenlik
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_float(c_ampersand_value *value, double val) {
    if (value == NULL || (value->type.kind != TYPE_FLOAT32 && value->type.kind != TYPE_FLOAT64)) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    if (value->type.kind == TYPE_FLOAT32) {
        value->data.float32_val = (float)val; // Hassasiyet kaybı olabilir
    } else { // TYPE_FLOAT64
        value->data.float64_val = val;
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_string(c_ampersand_value *value, const char *val) {
    if (value == NULL || value->type.kind != TYPE_STRING) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Mevcut string belleğini serbest bırak
    if (value->data.string_val.ptr != NULL) {
         // sahne_error_t sahne_err =
         sahne_mem_release(value->data.string_val.ptr, value->data.string_val.size);
         value->data.string_val.ptr = NULL;
         value->data.string_val.size = 0;
    }

    if (val == NULL) {
        // NULL string atanıyorsa sadece pointer'ı NULL yap
        return C_AMPERSAND_OK_RESULT;
    }

    // Stringin boyutunu al (null sonlandırıcı dahil)
    size_t len = strlen(val);
    size_t allocation_size = len + 1; // +1 null sonlandırıcı için

    void *new_ptr = NULL;
    // Yeni bellek tahsis et (Sahne64 API)
    sahne_error_t sahne_err = sahne_mem_allocate(allocation_size, &new_ptr);

    if (sahne_err != SAHNE_SUCCESS) {
        // Tahsis başarısız olursa OOM hatası dön
        value->data.string_val.ptr = NULL; // Hata durumunda pointer'ı NULL yap
        value->data.string_val.size = 0;
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
    }

    // Veriyi kopyala
    memcpy(new_ptr, val, allocation_size);

    // Yeni pointer ve boyutu kaydet
    value->data.string_val.ptr = (char *)new_ptr;
    value->data.string_val.size = allocation_size;

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_value_set_pointer(c_ampersand_value *value, void *val) {
    if (value == NULL || value->type.kind != TYPE_POINTER) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Pointer tipler genellikle belleği kendileri yönetmez, sadece işaret ederler.
    // Eğer pointer tipi yönetilen bir kaynağa işaret ediyorsa (örn. Handle gibi),
    // burada özel logic (resource_release gibi) gerekebilir.
    // Varsayım: Pointer sadece ham adresi saklar.
    value->data.pointer_val = val;
    return C_AMPERSAND_OK_RESULT;
}

// Get fonksiyonları - Null kontrolü dışında genellikle değişiklik yok
bool c_ampersand_value_get_bool(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_BOOL) {
        // Hata yönetimi eklenebilir (örn. loglama veya özel bir "geçersiz değer" döndürme)
        return false; // Varsayılan/güvenli bir değer dön
    }
    return value->data.bool_val;
}

int64_t c_ampersand_value_get_int(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind < TYPE_INT8 || value->type.kind > TYPE_INT64)) {
        // Hata yönetimi eklenebilir
        return 0; // Varsayılan/güvenli bir değer dön
    }
    // Hedef tipin union alanından değeri al
    switch (value->type.kind) {
        case TYPE_INT8: return value->data.i8_val;
        case TYPE_INT16: return value->data.i16_val;
        case TYPE_INT32: return value->data.i32_val;
        case TYPE_INT64: return value->data.i64_val;
        default: return 0; // Ulaşılamamalı
    }
}

uint64_t c_ampersand_value_get_uint(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind < TYPE_UINT8 || value->type.kind > TYPE_UINT64)) {
        // Hata yönetimi eklenebilir
        return 0; // Varsayılan/güvenli bir değer dön
    }
    // Hedef tipin union alanından değeri al
    switch (value->type.kind) {
        case TYPE_UINT8: return value->data.u8_val;
        case TYPE_UINT16: return value->data.u16_val;
        case TYPE_UINT32: return value->data.u32_val;
        case TYPE_UINT64: return value->data.u64_val;
        default: return 0; // Ulaşılamamalı
    }
}

double c_ampersand_value_get_float(const c_ampersand_value *value) {
    if (value == NULL || (value->type.kind != TYPE_FLOAT32 && value->type.kind != TYPE_FLOAT64)) {
        // Hata yönetimi eklenebilir
        return 0.0; // Varsayılan/güvenli bir değer dön
    }
    if (value->type.kind == TYPE_FLOAT32) {
        return (double)value->data.float32_val;
    } else { // TYPE_FLOAT64
        return value->data.float64_val;
    }
}

const char *c_ampersand_value_get_string(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_STRING) {
        // Hata yönetimi eklenebilir
        return NULL; // Varsayılan/güvenli bir değer dön
    }
    return value->data.string_val.ptr; // String pointer'ını döndür
}

void *c_ampersand_value_get_pointer(const c_ampersand_value *value) {
    if (value == NULL || value->type.kind != TYPE_POINTER) {
        // Hata yönetimi eklenebilir
        return NULL; // Varsayılan/güvenli bir değer dön
    }
    return value->data.pointer_val;
}

bool c_ampersand_value_equals(const c_ampersand_value *val1, const c_ampersand_value *val2) {
    // NULL veya farklı tipleri kontrol et
    if (val1 == NULL || val2 == NULL || val1->type.kind != val2->type.kind) {
        return false;
    }
    // Aynı tipler için değerleri karşılaştır
    switch (val1->type.kind) {
        case TYPE_BOOL:    return val1->data.bool_val == val2->data.bool_val;
        // Tamsayıları karşılaştırırken, union'ın en büyük ortak boyutunu kullanmak güvenli olabilir.
        // Ya da her bir tamsayı tipi için ayrı case yazılır. i64_val/u64_val kullanmak daha basit.
        case TYPE_INT8:
        case TYPE_INT16:
        case TYPE_INT32:
        case TYPE_INT64:   return val1->data.i64_val == val2->data.i64_val; // i64'e cast ederek karşılaştır
        case TYPE_UINT8:
        case TYPE_UINT16:
        case TYPE_UINT32:
        case TYPE_UINT64:  return val1->data.u64_val == val2->data.u64_val; // u64'e cast ederek karşılaştır
        // Float karşılaştırmaları hassasiyet sorunlarına dikkat etmeli (burada direkt == kullanıldı, genellikle kötü)
        case TYPE_FLOAT32: return val1->data.float32_val == val2->data.float32_val;
        case TYPE_FLOAT64: return val1->data.float64_val == val2->data.float64_val;
        case TYPE_STRING:
             // String pointerları veya içerikleri null olabilir
             if (val1->data.string_val.ptr == NULL && val2->data.string_val.ptr == NULL) return true; // İkisi de NULL ise eşit
             if (val1->data.string_val.ptr == NULL || val2->data.string_val.ptr == NULL) return false; // Biri NULL diğeri değilse eşit değil
             // İkisi de NULL değilse içeriği karşılaştır
             // Boyutları kontrol etmek ek güvenlik sağlar, ama strcmp null sonlandırıcıya bakar.
             // Basit strcmp yeterli, stringin doğru null-terminated olduğu varsayılıyor.
             return strcmp(val1->data.string_val.ptr, val2->data.string_val.ptr) == 0;

        case TYPE_POINTER: return val1->data.pointer_val == val2->data.pointer_val;
        // Diğer Sahne64'e özel tipler (örn. Handle) de burada karşılaştırılmalı
        // case TYPE_SAHNE64_HANDLE: return val1->data.handle_val == val2->data.handle_val;
        default:
             // Bilinmeyen veya void gibi karşılaştırılamayan tipler
             return false;
    }
}

// ... diğer veri tipi yönetimi ile ilgili fonksiyonlar eklenebilir

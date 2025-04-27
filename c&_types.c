#include "c&_types.h"
#include "c&_stdlib.h" // c_ampersand_println için
// Diğer modül başlıkları (örneğin, c&_memory.h eğer karmaşık tipler heap'te yaratılacaksa)
#include "c&_memory.h"

#include <string.h> // strcmp için (Sahne toolchain sağlıyorsa)
#include <stdbool.h> // bool için


// --- Temel Sabit Tür Tanımlamaları ---
// Bu singleton'lar statik bellekte yaşar.
// c_ampersand_type structı genişletildiğinde bu tanımlar güncellenmelidir.

const c_ampersand_type c_ampersand_type_unknown = { TYPE_UNKNOWN, {} }; // Boş union
const c_ampersand_type c_ampersand_type_void = { TYPE_VOID, {} };
const c_ampersand_type c_ampersand_type_bool = { TYPE_BOOL, {} };
const c_ampersand_type c_ampersand_type_int8 = { TYPE_INT8, {} };
const c_ampersand_type c_ampersand_type_int16 = { TYPE_INT16, {} };
const c_ampersand_type c_ampersand_type_int32 = { TYPE_INT32, {} };
const c_ampersand_type c_ampersand_type_int64 = { TYPE_INT64, {} };
const c_ampersand_type c_ampersand_type_uint8 = { TYPE_UINT8, {} };
const c_ampersand_type c_ampersand_type_uint16 = { TYPE_UINT16, {} };
const c_ampersand_type c_ampersand_type_uint32 = { TYPE_UINT32, {} };
const c_ampersand_type c_ampersand_type_uint64 = { TYPE_UINT64, {} };
const c_ampersand_type c_ampersand_type_float32 = { TYPE_FLOAT32, {} };
const c_ampersand_type c_ampersand_type_float64 = { TYPE_FLOAT64, {} };
const c_ampersand_type c_ampersand_type_string = { TYPE_STRING, {} };
// Pointer tipi özel bilgi gerektirir (işaret ettiği tip).
// Aşağıdaki tanım sadece kind'ı ayarlar, target_type NULL kalır.
// Gerçek pointer tipi oluşturma fonksiyonu (örn. c_ampersand_type_create_pointer) ile yapılmalıdır.
const c_ampersand_type c_ampersand_type_pointer = { TYPE_POINTER, {} };


// --- Tür Sistemi Başlangıç/Kapanış ---

// Tür sistemini başlatır (singleton temel türler zaten tanımlı).
c_ampersand_result c_ampersand_types_init() {
    // Statik const tanımlar zaten başlatılmış durumdadır.
    // Gelecekte dinamik tür kaydı veya başlatma gerekirse buraya eklenir.
    c_ampersand_println("TYPES BILGI: Tür sistemi başlatıldı.");
    return C_AMPERSAND_OK_RESULT;
}

// Tür sistemini kapatan fonksiyon (varsa ayrılan belleği serbest bırakır).
c_ampersand_result c_ampersand_types_shutdown() {
    // Statik const türler serbest bırakılmaz.
    // Gelecekte dinamik olarak yaratılan türler (örn. struct, array tipleri)
    // takip edilip burada serbest bırakılabilir.
    c_ampersand_println("TYPES BILGI: Tür sistemi kapatıldı.");
    return C_AMPERSAND_OK_RESULT;
}


// --- Tür Yardımcı Fonksiyonları ---

// Türlerin isimlerini döndüren fonksiyon (debug amaçlı).
const char *c_ampersand_type_to_string(c_ampersand_type type) {
    switch (type.kind) {
        case TYPE_UNKNOWN: return "unknown";
        case TYPE_VOID: return "void";
        case TYPE_BOOL: return "bool";
        case TYPE_INT8: return "i8";
        case TYPE_INT16: return "i16";
        case TYPE_INT32: return "i32";
        case TYPE_INT64: return "i64";
        case TYPE_UINT8: return "u8";
        case TYPE_UINT16: return "u16";
        case TYPE_UINT32: return "u32";
        case TYPE_UINT64: return "u64";
        case TYPE_FLOAT32: return "f32";
        case TYPE_FLOAT64: return "f64";
        case TYPE_STRING: return "string";
        case TYPE_POINTER: {
            // Pointer tipi için işaret ettiği tipi de göstermek daha iyi olabilir.
            // Ancak bu fonksiyon char* dönüyor, dinamik mesaj oluşturamaz.
            // Eğer hedef tipi belli ise:
             if (type.info.pointer_info.target_type != NULL) {
                static char buffer[64]; // Dikkat: Static buffer thread-safe DEĞİLDİR.
                snprintf(buffer, sizeof(buffer), "*%s", c_ampersand_type_to_string(*type.info.pointer_info.target_type));
                return buffer;
             }
            return "pointer"; // Basit gösterim
        }
        case TYPE_ARRAY: return "array"; // Detaylandırma gerekli
        case TYPE_STRUCT: return "struct"; // Detaylandırma gerekli
        case TYPE_FUNCTION: return "func_type"; // Detaylandırma gerekli
        default: return "invalid_type";
    }
}

// İki türün tamamen eşit olup olmadığını kontrol eder.
// Karmaşık tipler (pointer, array, struct, function) için info alanları da karşılaştırılmalıdır.
bool c_ampersand_type_equals(c_ampersand_type t1, c_ampersand_type t2) {
    if (t1.kind != t2.kind) {
        return false; // Temel tür türü farklı
    }

    // Aynı temel tür türündeyse, türün detaylarını karşılaştır
    switch (t1.kind) {
        case TYPE_POINTER:
            // İki pointer tipi ancak işaret ettikleri tipler aynıysa eşittir.
            // NULL pointer tipleri için özel durum (örn. void*).
            if (t1.info.pointer_info.target_type == NULL && t2.info.pointer_info.target_type == NULL) return true; // İkisi de void* gibi
            if (t1.info.pointer_info.target_type == NULL || t2.info.pointer_info.target_type == NULL) return false; // Biri NULL, diğeri değilse
            return c_ampersand_type_equals(*t1.info.pointer_info.target_type, *t2.info.pointer_info.target_type); // İşaret edilen tipleri rekürsif karşılaştır

        case TYPE_ARRAY:
            // İki dizi tipi, eleman tipleri aynıysa ve boyutları (sabitse) aynıysa eşittir.
            if (t1.info.array_info.array_size != t2.info.array_info.array_size) return false; // Boyut farklıysa
             if (t1.info.array_info.element_type == NULL && t2.info.array_info.element_type == NULL) return true; // Eleman tipi NULL ise (hatalı durum?)
             if (t1.info.array_info.element_type == NULL || t2.info.array_info.element_type == NULL) return false;
            return c_ampersand_type_equals(*t1.info.array_info.element_type, *t2.info.array_info.element_type); // Eleman tiplerini rekürsif karşılaştır

        case TYPE_STRUCT:
            // İki struct tipi, aynı struct tanımını temsil ediyorsa eşittir.
            // Bu genellikle struct'ın adını veya benzersiz bir ID'sini karşılaştırmak anlamına gelir.
            // Varsayım: struct_info.name benzersiz bir tanımlayıcıdır.
            if (t1.info.struct_info.name == NULL && t2.info.struct_info.name == NULL) return true;
            if (t1.info.struct_info.name == NULL || t2.info.struct_info.name == NULL) return false;
            // String karşılaştırması için c_ampersand_string_compare veya strcmp kullanılabilir.
            return strcmp(t1.info.struct_info.name, t2.info.struct_info.name) == 0; // İsimleri karşılaştır (Sahne toolchain strcmp sağlıyor varsayımıyla)

        case TYPE_FUNCTION:
            // İki fonksiyon tipi, dönüş tipleri aynıysa ve parametre tipleri aynıysa (sayı ve sıralama) eşittir.
            // Varsayım: function_info yeterli bilgiyi içeriyor ve parametre tipleri karşılaştırılabilir.
             if (!c_ampersand_type_equals(*t1.info.function_info.return_type, *t2.info.function_info.return_type)) return false;
            // // Parametre listelerini karşılaştır...
             return true; // Eğer dönüş tipi ve parametreler eşleşiyorsa

            // Placeholder olarak sadece kind eşitliği
            return true; // Fonksiyon tipi detay karşılaştırması implemente edilmedi

        case TYPE_STRING:
             // String tipi genellikle basittir, ek bilgi taşımaz (şimdilik).
             return true;

        default:
            // Basit tipler (void, bool, sayılar): Sadece kind eşitliği yeterli.
            return true;
    }
}

// Bir türün diğerine atanabilir/uyumlu olup olmadığını kontrol eder (örn. int -> float).
// Bu, dilin örtük dönüşüm kurallarına göre implemente edilmelidir.
bool c_ampersand_type_is_compatible(c_ampersand_type from, c_ampersand_type to) {
    // Tam eşitlik her zaman uyumluluktur.
    if (c_ampersand_type_equals(from, to)) {
        return true;
    }

    // Temel sayısal tür dönüşümleri (genişletme)
    // Örnek: int8 -> int16, int -> float, uint -> int (dikkatli), float32 -> float64
    // Tam kurallar dil spesifikasyonuna bağlıdır.
    if (from.kind >= TYPE_INT8 && from.kind <= TYPE_UINT64 &&
        to.kind >= TYPE_INT8 && to.kind <= TYPE_UINT64)
    {
        // Sayısal tipler arası dönüşüm kontrolü.
        // Örneğin, daha küçük bir tam sayı daha büyük bir tam sayıya atanabilir.
        // İşaretli/işaretsiz dönüşümler karmaşıktır.
        // float ile tam sayı arası dönüşümler de uyumlu olabilir.
        // Bu kısım gerçek dönüşüm kurallarına göre detaylandırılmalıdır.
        c_ampersand_println("TYPES NOT: Sayısal tip uyumluluk kontrolü placeholder.");
        // Geçici olarak sadece int/uint/float arası her şeyi uyumlu varsayalım (ÇOK YANLIŞ!)
        return true; // PLACEHOLDER
    }

    // Pointer uyumluluğu (örn. T* -> const T*, T* -> void*)
    if (from.kind == TYPE_POINTER && to.kind == TYPE_POINTER) {
        c_ampersand_println("TYPES NOT: Pointer tip uyumluluk kontrolü placeholder.");
         // Örneğin: from.info.pointer_info.target_type T, to.info.pointer_info.target_type const T ise uyumlu olabilir.
         // void* her pointer tipiyle uyumlu olabilir.
         return true; // PLACEHOLDER
    }

    // Diğer özel uyumluluklar (örn. C-string -> C& string)
     if (from.kind == TYPE_C_STRING && to.kind == TYPE_STRING) return true; // Eğer C-string tipi varsa

    // Farklı tür türleri genellikle uyumlu değildir (int = struct yanlıştır).
    return false; // Varsayılan: Uyumsuz
}

// ... Diğer tür yönetimi fonksiyonları implementasyonu (örn. c_ampersand_type_create_pointer, c_ampersand_type_create_array vb.)
// Bu fonksiyonlar heap'te c_ampersand_type structları yaratmalı ve c&_types_shutdown'da serbest bırakılmak üzere takip edilmelidir.
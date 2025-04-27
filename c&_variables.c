#include "c&_variables.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h" // c_ampersand_result, c_ampersand_error_with_message, c_ampersand_println için
// Veri tipleri tanımı için
#include "c&_types.h" // c_ampersand_type struct, c_ampersand_type_equals (varsayım)
// Değerleri tutmak için
#include "c&_data_types.h" // c_ampersand_value struct, c_ampersand_value_create, c_ampersand_value_free, c_ampersand_value_copy (varsayım)
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h" // c_ampersand_allocate, c_ampersand_free
// C& string fonksiyonları için
#include "c&_string.h" // c_ampersand_string_compare

#include <stdbool.h>
#include <stddef.h> // size_t için
#include <string.h> // memset, memcpy için (Sahne toolchain sağlıyor varsayımıyla)


// Global scope tanımı
c_ampersand_scope *global_scope = NULL;


// --- Değişken Yönetimi ---

// Yeni bir değişken oluşturur. Ad stringini ve değişken struct'ını C& belleğinde tahsis eder.
c_ampersand_variable *c_ampersand_variable_create(const char *name, c_ampersand_type type) {
    // Parametre doğrulama
    if (name == NULL || strlen(name) == 0) {
        c_ampersand_println("VARIABLES HATA: variable_create NULL veya boş isimle çağrıldı.");
        return NULL; // Hata durumunda NULL pointer dönmek uygun
    }
    // Tip validity check de yapılabilir.

    c_ampersand_variable *var = NULL;
    size_t var_struct_size = sizeof(c_ampersand_variable);
    // Değişken structı için bellek tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(var_struct_size, (void**)&var);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği, hata c_ampersand_allocate içinde raporlanır.
        return NULL; // NULL döndür
    }

    // Tahsis edilen belleği sıfırla
    memset(var, 0, var_struct_size);

    // Değişken adı stringi için bellek tahsis et ve kopyala
    size_t name_len = strlen(name);
    size_t name_alloc_size = name_len + 1; // Null sonlandırıcı dahil
    char *name_ptr = NULL;
    alloc_res = c_ampersand_allocate(name_alloc_size, (void**)&name_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Ad stringi tahsis hatası, değişken struct'ını serbest bırak
        c_ampersand_free(var, var_struct_size); // Variable structını free et
        return NULL; // NULL döndür
    }

    // Ad stringini kopyala
    memcpy(name_ptr, name, name_alloc_size);

    // Değişken alanlarını doldur
    var->name = name_ptr;
    var->name_allocated_size = name_alloc_size; // Ad stringi boyutu
    var->type = type; // Tip structı kopyalanır
    var->variable_allocated_size = var_struct_size; // Variable struct boyutu

    // Değişkenin değeri için bellek ve başlangıç değeri oluştur (c&_data_types.h'den fonksiyon)
    c_ampersand_result value_create_res = c_ampersand_value_create(type, &var->value); // var->value'yu doldurur (value.value union'ı içinde)

    if (value_create_res.code != C_AMPERSAND_OK) {
        // Değer oluşturma hatası (bellek veya geçersiz tip vb.)
        // Hata mesajını c_ampersand_value_create'den alır (varsayım)
        c_ampersand_println("VARIABLES HATA: '%s' değişkeni için değer oluşturulurken hata: %s", name, value_create_res.message);
        // c_ampersand_result_print_error(value_create_res);
        c_ampersand_result_free_message(&value_create_res); // Mesajı serbest bırak

        // Daha önce tahsis edilen belleği serbest bırak
        c_ampersand_free(var->name, var->name_allocated_size); // Ad stringini free et
        c_ampersand_free(var, var->variable_allocated_size); // Variable structını free et
        return NULL; // NULL döndür
    }

    var->next = NULL; // Bağlı liste için next pointer

    return var; // Başarı
}

// Bir değişkeni serbest bırakır. Ad stringini, değeri ve değişken struct'ını serbest bırakır.
// c_ampersand_free ve c_ampersand_value_free kullanır. Hataları loglar.
void c_ampersand_variable_free(c_ampersand_variable *var) {
    if (var == NULL) {
        return; // NULL pointer güvenliği
    }

    // Değişken adını serbest bırak
    if (var->name != NULL && var->name_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(var->name, var->name_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
             c_ampersand_println("VARIABLES UYARI: '%s' değişken adı serbest bırakılırken hata (%d).", var->name ? var->name : "NULL", free_res.code);
             // c_ampersand_result_print_error(free_res);
             c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
        var->name = NULL;
        var->name_allocated_size = 0;
    } else if (var->name != NULL && var->name_allocated_size == 0) {
         c_ampersand_println("VARIABLES UYARI: '%s' değişken adı pointerı NULL değil ama boyut 0.", var->name);
    }


    // Değişken değerini serbest bırak (c&_data_types.h'den fonksiyon)
    // Bu, value structı içindeki heap belleği (örn. string için) serbest bırakır.
    c_ampersand_result value_free_res = c_ampersand_value_free(&var->value);
    if (value_free_res.code != C_AMPERSAND_OK) {
         c_ampersand_println("VARIABLES UYARI: '%s' değişken değeri serbest bırakılırken hata (%d).", var->name ? var->name : "NULL", value_free_res.code);
         // c_ampersand_result_print_error(value_free_res);
         c_ampersand_result_free_message(&value_free_res); // Mesajı serbest bırak
    }
    // var->value alanını sıfırlamak gerekli olmayabilir, struct free edilecek.


    // Değişken struct'ını serbest bırak
    if (var->variable_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(var, var->variable_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
             c_ampersand_println("VARIABLES UYARI: '%s' değişken structı serbest bırakılırken hata (%d).", var->name ? var->name : "NULL", free_res.code);
             // c_ampersand_result_print_error(free_res);
             c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
    } else {
         c_ampersand_println("VARIABLES UYARI: Free edilmek istenen değişken structı allocated_size 0.");
    }
}

// Bir değişkene değer atar (runtime'da). Değerin derin kopyasını yapabilir (c_ampersand_value_copy kullanarak).
c_ampersand_result c_ampersand_variable_set_value(c_ampersand_variable *var, const c_ampersand_value *value) {
    // Parametre doğrulama
    if (var == NULL || value == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "variable_set_value: Geçersiz argüman (NULL pointer).");
    }
    // Tip uyumluluğu kontrolü (temel kind eşitliği veya daha karmaşık uyumluluk)
    // Varsayım: c_ampersand_type_equals veya c_ampersand_type_is_compatible c&_types.h/c'de tanımlı.
    // Şimdilik sadece kind eşitliği kontrol edelim (mevcut koda benzer).
    if (var->type.kind != value->type.kind) {
        // Gelişmiş tip uyumluluğu kontrolü gerekebilir (örn. int = float).
        // bool types_compatible = c_ampersand_type_is_compatible(var->type, value->type);
        // if (!types_compatible) {
        // ... hata mesajına tip isimlerini de ekleyebiliriz ...
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_TYPE_MISMATCH, "variable_set_value: Uyumsuz tip ataması."); // Mesaj tip detayını içerebilir
        // }
    }

    // Değişkenin mevcut değerini serbest bırak (eğer heap belleği kullanıyorsa - örn. string).
    // c_ampersand_value_free, value'nun içeriğini türüne göre serbest bırakır.
    c_ampersand_result free_old_value_res = c_ampersand_value_free(&var->value);
    if (free_old_value_res.code != C_AMPERSAND_OK) {
        // Eski değeri serbest bırakırken hata, logla ama işleme devam et? Kritik hata olabilir.
         c_ampersand_println("VARIABLES UYARI: '%s' değişkenine değer atanırken eski değer serbest bırakılamadı (%d).", var->name ? var->name : "NULL", free_old_value_res.code);
         c_ampersand_result_free_message(&free_old_value_res);
         // İşleme devam etmek riskli, bellek sızıntısı olabilir. Hata dönmek daha güvenli.
         return free_old_value_res; // Serbest bırakma hatasını ilet
    }
    // var->value şimdi sıfırlanmış durumda.

    // Yeni değeri değişkene kopyala (derin kopya c_ampersand_value_copy tarafından yapılır)
    // c_ampersand_value_copy c&_data_types.h/c'de tanımlı varsayılıyor.
    c_ampersand_result copy_res = c_ampersand_value_copy(&var->value, value);

    if (copy_res.code != C_AMPERSAND_OK) {
        // Değer kopyalama hatası (bellek yetersizliği vb.)
        // c_ampersand_value_copy zaten hata mesajını döner (varsayım).
        return copy_res; // Kopyalama hatasını ilet
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir değişkenin değerini alır (runtime'da). Değerin derin kopyasını out_value'ya yapar.
// out_value'nun heap belleği bu fonksiyon içinde tahsis edilebilir.
c_ampersand_result c_ampersand_variable_get_value(const c_ampersand_variable *var, c_ampersand_value *out_value) {
    // Parametre doğrulama
    if (var == NULL || out_value == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "variable_get_value: Geçersiz argüman (NULL pointer).");
    }
    // out_value yapısını başlangıçta sıfırla (c_ampersand_value_copy'nin doğru çalışması için)
    memset(out_value, 0, sizeof(c_ampersand_value));

    // Değişkenin değerini out_value'ya kopyala (derin kopya c_ampersand_value_copy tarafından yapılır)
    // c_ampersand_value_copy c&_data_types.h/c'de tanımlı varsayılıyor.
    c_ampersand_result copy_res = c_ampersand_value_copy(out_value, &var->value);

     if (copy_res.code != C_AMPERSAND_OK) {
        // Değer kopyalama hatası (bellek yetersizliği vb.)
        // c_ampersand_value_copy zaten hata mesajını döner (varsayım).
        // out_value kısmen doldurulmuş/bellek sızıntısı olabilir, temizlemek gerekebilir.
        c_ampersand_value_free(out_value); // Kopyalama başarısız olursa out_value'yu temizle.
        return copy_res; // Kopyalama hatasını ilet
    }

    // Çağıran out_value'yu kullanmayı bitirdiğinde c_ampersand_value_free ile serbest bırakmalıdır.
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Belirtilen scope'ta belirli bir ada sahip bir değişkeni arar (scope hiyerarşisinde yukarı çıkarak).
c_ampersand_variable *c_ampersand_variable_lookup(c_ampersand_scope *scope, const char *name) {
    // Parametre doğrulama
    if (scope == NULL || name == NULL || strlen(name) == 0) {
        // Geçersiz argüman, loglama yapılabilir veya NULL dönülebilir.
        c_ampersand_println("VARIABLES UYARI: variable_lookup NULL veya boş isimle çağrıldı.");
        return NULL;
    }

    // Mevcut scope'tan başlayarak üst scopelara doğru ara
    c_ampersand_scope *current_scope = scope;
    while (current_scope != NULL) {
        // Mevcut scope'taki değişken listesini gez
        c_ampersand_variable *current_var = current_scope->variables;
        while (current_var != NULL) {
            // Değişken adlarını karşılaştır
            if (current_var->name != NULL && c_ampersand_string_compare(current_var->name, name) == 0) { // c_ampersand_string_compare kullan
                return current_var; // Değişken bulundu
            }
            current_var = current_var->next; // Sonraki değişkene geç
        }
        // Mevcut scope'ta bulunamadı, üst scope'a geç
        current_scope = current_scope->parent; // Üst scope bağlantısını kullan
    }

    // Değişken hiçbir scope'ta bulunamadı
    return NULL;
}

// --- Scope Yönetimi ---

// Yeni bir scope oluşturur. Scope struct'ını C& belleğinde tahsis eder.
c_ampersand_scope *c_ampersand_scope_create(c_ampersand_scope *parent) {
    c_ampersand_scope *scope = NULL;
    size_t scope_struct_size = sizeof(c_ampersand_scope);
    // Scope structı için bellek tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(scope_struct_size, (void**)&scope);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği, hata c_ampersand_allocate içinde raporlanır.
        return NULL; // NULL döndür
    }

    // Tahsis edilen belleği sıfırla
    memset(scope, 0, scope_struct_size);

    // Alanları ayarla
    scope->variables = NULL;
    scope->parent = parent; // Üst scope'u kaydet
    scope->scope_allocated_size = scope_struct_size; // Tahsis edilen boyutu kaydet

    return scope; // Başarı
}

// Bir scope'u ve içindeki tüm değişkenleri rekürsif olarak serbest bırakır.
// c_ampersand_free ve c_ampersand_variable_free kullanır. Hataları loglar.
void c_ampersand_scope_free(c_ampersand_scope *scope) {
    if (scope == NULL) {
        return; // NULL pointer güvenliği
    }

    // Scope içindeki değişkenleri serbest bırak (bağlı listeyi gez)
    c_ampersand_variable *current = scope->variables;
    while (current != NULL) {
        c_ampersand_variable *next = current->next;
        c_ampersand_variable_free(current); // Değişken structı ve içeriğini serbest bırakır (hataları loglar)
        current = next; // Sonraki değişkene geç
    }
    scope->variables = NULL; // Liste başını NULL yap

    // Scope struct'ını serbest bırak
    if (scope->scope_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(scope, scope->scope_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
             c_ampersand_println("VARIABLES UYARI: Scope structı serbest bırakılırken hata (%d).", free_res.code);
             // c_ampersand_result_print_error(free_res);
             c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
    } else {
         c_ampersand_println("VARIABLES UYARI: Free edilmek istenen scope structı allocated_size 0.");
    }

    // Not: Üst scope (parent) bu fonksiyon tarafından serbest bırakılmaz.
    // Scope hiyerarşisi yönetimi (oluşturma, serbest bırakma sırası) çağıranın sorumluluğundadır (örn. interpreter).
}

// Belirtilen scope'a bir değişken ekler. Değişkenin aynı scope'ta zaten tanımlı olup olmadığını kontrol eder.
c_ampersand_result c_ampersand_scope_add_variable(c_ampersand_scope *scope, c_ampersand_variable *var) {
    // Parametre doğrulama
    if (scope == NULL || var == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "scope_add_variable: Geçersiz argüman (NULL scope veya variable).");
    }

    // Değişkenin adının geçerli olup olmadığını kontrol et
    if (var->name == NULL || strlen(var->name) == 0) {
         // Bu normalde variable_create içinde kontrol edilmeli, ama burada da kontrol etmek iyi olur.
         c_ampersand_println("VARIABLES HATA: scope_add_variable eklenmek istenen değişkenin adı NULL veya boş.");
         return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "scope_add_variable: Eklenmek istenen değişkenin adı geçersiz.");
    }

    // Aynı scope içinde aynı isme sahip başka bir değişken var mı kontrol et
    // Sadece mevcut scope'un listesini gezmek yeterlidir.
    c_ampersand_variable *current_var = scope->variables;
    while (current_var != NULL) {
        if (current_var != var && // Kendisiyle karşılaştırma yapma
            current_var->name != NULL && c_ampersand_string_compare(current_var->name, var->name) == 0) { // c_ampersand_string_compare kullan
            // Aynı isimde başka bir değişken bulundu
             c_ampersand_println("VARIABLES HATA: Scope'a zaten tanımlı değişken eklendi: %s", var->name);
            return c_ampersand_error_with_message(C_AMPERSAND_ERROR_SEMANTIC, "scope_add_variable: Değişken '%s' mevcut scope'ta zaten tanımlı.", var->name);
            // Not: Bu bir runtime/interpreter hatasıdır. Compile-time semantic analysis'teki symbol table kontrolü farklıdır.
        }
        current_var = current_var->next;
    }


    // Değişkeni scope'un değişken listesinin başına ekle
    var->next = scope->variables;
    scope->variables = var;

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// --- Başlangıç ve Kapanış ---

// Değişken yönetim modülünü başlatır (global scope'u oluşturur).
c_ampersand_result c_ampersand_variables_init() {
    // Global scope daha önce oluşturulmuş mu kontrol et
    if (global_scope == NULL) {
        // Global scope'u oluştur (üst scope'u NULL)
        global_scope = c_ampersand_scope_create(NULL);
        if (global_scope == NULL) {
            // Oluşturma hatası (bellek yetersizliği) c_ampersand_scope_create içinde loglanır/raporlanır.
            // Sadece hata result'ı dönelim.
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, c_ampersand_error_with_message(C_AMPERSAND_ERROR_OUT_OF_MEMORY, "Değişken modülü başlatılırken global scope oluşturulamadı.").message, 0}; // Mesajlı OOM
        }
        c_ampersand_println("VARIABLES BILGI: Değişken modülü başlatıldı, global scope oluşturuldu.");
        return C_AMPERSAND_OK_RESULT; // Başarı
    }
    // Zaten başlatılmışsa başarı dön
    c_ampersand_println("VARIABLES BILGI: Değişken modülü zaten başlatılmış.");
    return C_AMPERSAND_OK_RESULT;
}

// Değişken yönetim modülünü kapatır (global scope'u ve içindeki her şeyi serbest bırakır).
c_ampersand_result c_ampersand_variables_shutdown() {
    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Global scope varsa serbest bırak
    if (global_scope != NULL) {
        // Global scope'u serbest bırak (içindeki tüm değişkenleri de serbest bırakır)
        c_ampersand_scope_free(global_scope); // free_scope içindeki hatalar orada loglanır.
        // scope_free void döndürdüğü için direkt sonucu alamayız, loglamaya güveniyoruz.
        global_scope = NULL; // Global scope pointerını NULL yap
        c_ampersand_println("VARIABLES BILGI: Değişken modülü kapatıldı, global scope serbest bırakıldı.");
    } else {
         c_ampersand_println("VARIABLES BILGI: Değişken modülü zaten kapatılmış.");
    }

    // Global scope serbest bırakma hatası varsa nasıl bileceğiz? scope_free void döndürüyor.
    // Daha sağlam bir tasarımda, scope_free de c_ampersand_result döndürmeliydi.
    // Şimdilik loglamaya güveniyoruz ve shutdown genellikle kritik olduğu için genel sonucu hep OK dönelim.
    // Veya loglanan hataların sayısını takip edip genel sonucu belirleyebiliriz.

    return final_result; // Başarı (veya loglanan hatalar olduysa bile OK döner)
}

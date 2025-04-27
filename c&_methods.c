#include "c&_methods.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println için
#include "c&_stdlib.h"
// C& veri tipleri için
#include "c&_types.h"
// C& değer yapıları ve string value formatı için
#include "c&_data_types.h"
// C& bellek yönetimi fonksiyonları için (Sahne64 API sarmalayıcıları)
#include "c&_memory.h"
#include <string.h> // strlen, strcmp için (snprintf de içerebilir)
#include <stdio.h>  // snprintf için


// Tüm metot tablolarının bağlı listesinin başı (global tanım)
c_ampersand_method_table *global_method_table = NULL;

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
 static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);

// --- Metot Yönetimi ---

// Yeni bir metot yapısı oluşturur ve pointer'ını out_method'a yazar.
// c_ampersand_allocate kullanır.
c_ampersand_result c_ampersand_method_create(const char *name, c_ampersand_type_kind type, c_ampersand_method_func func_ptr, c_ampersand_method **out_method) {
    if (name == NULL || func_ptr == NULL || out_method == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_method *method = NULL;
    size_t method_struct_size = sizeof(c_ampersand_method);
    // Metot struct'ı için bellek tahsis et (C& bellek API)
    c_ampersand_result alloc_res = c_ampersand_allocate(method_struct_size, (void**)&method);
    if (alloc_res.code != C_AMPERSAND_OK) {
        *out_method = NULL; // Hata durumunda çıktı pointer'ı NULL yap
        return alloc_res; // Bellek hatasını ilet
    }

    // Tahsis edilen belleği sıfırla (güvenlik için)
    memset(method, 0, method_struct_size);

    // Metot adı stringi için bellek tahsis et (C& bellek API)
    size_t name_len = strlen(name);
    size_t name_allocation_size = name_len + 1; // Null sonlandırıcı dahil
    char *name_ptr = NULL;
    alloc_res = c_ampersand_allocate(name_allocation_size, (void**)&name_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // String bellek hatası, daha önce tahsis edilen metot struct'ını serbest bırak
        // c_ampersand_free, boyutu da aldığı için bu boyut bilgisini kaydetmiştik.
        // method->allocated_size = method_struct_size; // create sırasında bu bilgiyi kaydetmek gerekiyordu.
        // Şimdilik basitleştirip struct boyutunu direkt geçelim.
        c_ampersand_free(method, method_struct_size); // Hatanın dönüş değerini kontrol etmiyoruz (kritik hata olabilir)
        *out_method = NULL;
        return alloc_res; // Bellek hatasını ilet
    }

    // Stringi kopyala
    memcpy(name_ptr, name, name_allocation_size);

    // Metot yapısı alanlarını ayarla
    method->name = name_ptr;
    method->name_allocated_size = name_allocation_size; // Ad için ayrılan boyutu kaydet

    method->belongs_to_type = type;
    method->func_ptr = func_ptr;
    method->next = NULL; // Başlangıçta bağlı liste boş
    method->allocated_size = method_struct_size; // Struct için ayrılan boyutu kaydet

    *out_method = method; // Oluşturulan pointer'ı çıktı parametresine yaz

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir metot yapısını ve içerdiği string belleğini serbest bırakır.
// c_ampersand_free kullanır.
c_ampersand_result c_ampersand_method_free(c_ampersand_method *method) {
    if (method == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL pointer'ı serbest bırakmak güvenlidir
    }

    c_ampersand_result free_name_res = C_AMPERSAND_OK_RESULT;
    // Metot adı stringini serbest bırak (eğer tahsis edildiyse)
    if (method->name != NULL) {
        free_name_res = c_ampersand_free((void*)method->name, method->name_allocated_size);
        // Serbest bırakma hatası olursa, devam etmeye çalışırız ama hatayı kaydederiz.
        method->name = NULL; // Pointer'ı NULL yap
        method->name_allocated_size = 0;
    }

    c_ampersand_result free_method_res = C_AMPERSAND_OK_RESULT;
    // Metot struct'ının kendisini serbest bırak
    // allocated_size alanı create sırasında ayarlanmıştı.
    if (method->allocated_size > 0) {
         free_method_res = c_ampersand_free(method, method->allocated_size);
         // Serbest bırakma hatası olursa, devam etmeye çalışırız ama hatayı kaydederiz.
    } else {
        // allocated_size 0 ise, ya bu metot heap'te ayrılmadı ya da bir hata var.
        // Bu durumda free yapmamak veya bir hata dönmek daha güvenli olabilir.
        // Şu anki tasarımda create her zaman allocate yapıp size ayarlar.
        // Eğer buraya allocated_size 0 olan bir şey geliyorsa bu bir programlama hatasıdır.
        // return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Hata durumu
    }


    // İki free işleminden herhangi birinde hata olduysa ilk hatayı döndür
    if (free_name_res.code != C_AMPERSAND_OK) return free_name_res;
    return free_method_res; // Name serbest bırakma başarılıysa, method serbest bırakma sonucunu dön
}

// Belirli bir tipe bir metot ekler.
// c_ampersand_allocate (yeni tablo için) kullanır.
c_ampersand_result c_ampersand_method_register(c_ampersand_type_kind type, c_ampersand_method *method) {
    if (method == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL && current_table->type != type) {
        current_table = current_table->next;
    }

    if (current_table == NULL) {
        // Bu tip için henüz bir metot tablosu yok, oluştur
        c_ampersand_method_table *new_table = NULL;
        size_t table_size = sizeof(c_ampersand_method_table);
        // Yeni tablo struct'ı için bellek tahsis et (C& bellek API)
        c_ampersand_result alloc_res = c_ampersand_allocate(table_size, (void**)&new_table);

        if (alloc_res.code != C_AMPERSAND_OK) {
            return alloc_res; // Bellek hatasını ilet
        }

        // Tahsis edilen belleği sıfırla
        memset(new_table, 0, table_size);

        // Yeni tablo alanlarını ayarla
        new_table->type = type;
        new_table->methods = method; // Yeni metot listenin ilk elemanı olur
        new_table->next = global_method_table; // Yeni tabloyu global listenin başına ekle
        global_method_table = new_table;
        new_table->allocated_size = table_size; // Ayrılan boyutu kaydet

    } else {
        // Mevcut tabloya metodu ekle (listenin başına ekle)
        // Metot create edildiğinde next alanı NULL'dur. Burada listenin başına eklenir.
        method->next = current_table->methods;
        current_table->methods = method;
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir nesne ve metot adına göre metodu arar.
// Bellek ayırmaz veya serbest bırakmaz.
c_ampersand_method *c_ampersand_method_lookup(c_ampersand_type_kind object_type, const char *method_name) {
    if (method_name == NULL) {
        return NULL; // Metot adı NULL olamaz
    }

    // Global metot tabloları listesinde tipi ara
    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL && current_table->type != object_type) {
        current_table = current_table->next;
    }

    // Tipin tablosu bulunduysa, metotları arasında adı ara
    if (current_table != NULL) {
        c_ampersand_method *current_method = current_table->methods;
        while (current_method != NULL) {
            // Metot adı geçerli mi kontrol et
            if (current_method->name != NULL) {
                if (strcmp(current_method->name, method_name) == 0) {
                    return current_method; // Metot bulundu
                }
            }
            current_method = current_method->next;
        }
    }

    return NULL; // Metot veya tablo bulunamadı
}

// Bir nesne üzerinde bir metodu çağırır.
// Metot lookup yapar ve fonksiyon pointer'ı çağırır.
// Hata mesajı için c_ampersand_error_with_message kullanır.
c_ampersand_result c_ampersand_method_call(c_ampersand_value *object, const char *method_name, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    // Gerekli parametreleri doğrula
    if (object == NULL || method_name == NULL || result == NULL) { // args ve num_args NULL olabilir (0 argüman)
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Çağrı sonucunu saklamak için result value'yu sıfırla veya varsayılan yap
    // (Bu metot çağırma konseptine bağlı olarak değişir, belki caller tarafından sıfırlanır)
    // Basitçe sıfırlayalım:
    memset(result, 0, sizeof(c_ampersand_value));
    // Veya result->type = TYPE_UNKNOWN; gibi bir başlangıç değeri atayabiliriz.

    // Nesnenin tipine ve metot adına göre metodu ara
    c_ampersand_method *method = c_ampersand_method_lookup(object->type.kind, method_name);
    if (method == NULL) {
        // Metot bulunamadı hatası oluştur (mesaj stringi allocate edilir)
        // Varsayım: C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION c&_stdlib.h'de tanımlı
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Metot bulunamadı: %s (Tip: %d)", method_name, object->type.kind);
    }

    // Metodu çağır (fonksiyon pointer'ı üzerinden)
    return method->func_ptr(object, args, num_args, result); // Metot implementasyonunun dönüş değerini direkt ilet
}

// --- Başlangıç ve Kapanış ---

// Metot sistemini (global tablo başını) başlatır.
c_ampersand_result c_ampersand_methods_init() {
    global_method_table = NULL; // Global metot tablosu başını NULL yap
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Metot sistemini (global tabloyu ve tüm metotları) kapatır ve kullanılan belleği serbest bırakır.
// c_ampersand_method_free ve c_ampersand_free kullanır.
c_ampersand_result c_ampersand_methods_shutdown() {
    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    c_ampersand_method_table *current_table = global_method_table;
    while (current_table != NULL) {
        c_ampersand_method_table *next_table = current_table->next; // Bir sonraki tablo pointer'ını kaydet

        // Tablodaki tüm metotları serbest bırak
        c_ampersand_method *current_method = current_table->methods;
        while (current_method != NULL) {
            c_ampersand_method *next_method = current_method->next; // Bir sonraki metot pointer'ını kaydet
            // Metodu serbest bırak (adı ve struct'ı)
            c_ampersand_result free_res = c_ampersand_method_free(current_method);
            if (free_res.code != C_AMPERSAND_OK) {
                // Metot serbest bırakmada hata olursa logla ve hatayı kaydet
                // c_ampersand_result_print_error(free_res); // Hata detayını yazdır
                if (final_result.code == C_AMPERSAND_OK) final_result = free_res; // İlk hatayı sakla
                // c_ampersand_result_free(&free_res); // Mesajı serbest bırak (eğer varsa)
            }

            current_method = next_method; // Bir sonraki metoda geç
        }
        // Tablonun metot listesi tamamlandı, tablo struct'ını serbest bırak
        c_ampersand_result free_table_res = C_ampersand_free(current_table, current_table->allocated_size);
         if (free_table_res.code != C_AMPERSAND_OK) {
            // Tablo serbest bırakmada hata olursa logla ve hatayı kaydet
            // c_ampersand_result_print_error(free_table_res);
            if (final_result.code == C_AMPERSAND_OK) final_result = free_table_res; // İlk hatayı sakla
            // c_ampersand_result_free(&free_table_res); // Mesajı serbest bırak (eğer varsa)
        }


        current_table = next_table; // Bir sonraki tabloya geç
    }

    global_method_table = NULL; // Global başı NULL yap
    return final_result; // Kapanış işleminin genel sonucunu dön
}


// --- Örnek Metot Implementasyonları ---

// string.length() metodu
// self: string c_ampersand_value
// args: argümanlar (beklenti: 0 argüman)
// result: i64 c_ampersand_value olarak sonucu döndürür
c_ampersand_result string_length(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    // self ve result pointerlarını kontrol et
    if (self == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // 'self' nesnesinin tipini doğrula
    if (self->type.kind != TYPE_STRING) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    // Argüman sayısını doğrula
    if (num_args != 0) {
        // Argüman hatası için mesajlı hata dön
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "Metot 'length' argüman almaz (verilen: %zu).", num_args);
    }

    // String uzunluğunu hesapla ve sonuç value'ya yaz
    // c_ampersand_value string değeri ptr/size structı olarak saklar varsayımıyla
    // strlen kullanmak yerine bu size alanını kullanmak daha doğru.
    // Eğer c_ampersand_value string_val ptr/size structı değilse, strlen kullanmak gerekir.
    // Varsayım: c_ampersand_value string_val bir struct { char* ptr; size_t size; }
    // Ancak buradaki kod self->data.string_val'in char* olduğunu varsayıyor ve strlen kullanıyor.
    // Önceki güncellemelerde string_val'ı struct yapmıştık. Bu implementasyonu struct'a göre düzeltelim.
    // Düzeltme: Original code used strlen on self->data.string_val assuming char*. Let's use the size field from our updated c_ampersand_value.
    // strlen(self->data.string_val) --> self->data.string_val.size (eğer null sonlandırıcı dahil değilse strlen gerekli)
    // Varsayım: c_ampersand_value::string_val struct { char* ptr; size_t size; }
    // Varsayım: size alanı null sonlandırıcı dahil değil. strlen gereklidir.
    // Varsayım: strlen Sahne64/C& stdlib'de mevcut.

    size_t string_len = strlen(self->data.string_val.ptr); // Düzeltme: Updated c_ampersand_value struct access

    // Sonuç value'yu ayarla
    // Varsayım: c_ampersand_type_int64 c&_types.h'de tanımlı bir tip objesi/struct'ıdır.
    // Veya c_ampersand_type_int64 bir c_ampersand_type objesi döndüren fonksiyondur.
    // Ya da c_ampersand_value::type bir c_ampersand_type_kind enum'ıdır.
    // Önceki güncellemeler c_ampersand_value::type'ın c_ampersand_type structı olduğunu ima etti.
    // Ve c_ampersand_type structı içinde kind alanı var. self->type.kind kullanıldı.
    // result->type da c_ampersand_type structı olmalı. Ya global/const bir c_ampersand_type objesi
    // atanmalı (c_ampersand_type_int64 gibi) ya da bir fonksiyondan gelmeli.
    // Varsayım: c_ampersand_type_int64, c_ampersand_type structına sahip bir const global veya fonksiyondur.
    result->type = c_ampersand_type_int64; // Varsayım: c_ampersand_type_int64 geçerli bir tip değeri/struct'ıdır.
    result->data.i64_val = (int64_t)string_len; // size_t'den int64_t'ye cast

    // Sonuç value'nun diğer alanlarını (varsa, örn. is_gc_managed) ayarla
    // result->is_gc_managed = false; // Sayısal değer GC yönetimi gerektirmez

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// int.to_string(), long.to_string() metodu
// self: int32 veya int64 c_ampersand_value
// args: argümanlar (beklenti: 0 argüman)
// result: string c_ampersand_value olarak sonucu döndürür (yeni string tahsis edilir)
c_ampersand_result int_to_string(c_ampersand_value *self, const c_ampersand_value *args, size_t num_args, c_ampersand_value *result) {
    // self ve result pointerlarını kontrol et
    if (self == NULL || result == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // 'self' nesnesinin tipini doğrula
    if (self->type.kind != TYPE_INT32 && self->type.kind != TYPE_INT64) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
    }

    // Argüman sayısını doğrula
    if (num_args != 0) {
        // Argüman hatası için mesajlı hata dön
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "Metot 'to_string' argüman almaz (verilen: %zu).", num_args);
    }

    // Sayıyı stringe çevir
    char buffer[64]; // Sayıyı tutacak yeterli boyutlu stack buffer (taşma riskine dikkat!)
    int chars_written;

    if (self->type.kind == TYPE_INT32) {
        // snprintf ile int32'yi buffer'a formatla
        // Varsayım: snprintf Sahne64/C& stdlib'de mevcut.
        chars_written = snprintf(buffer, sizeof(buffer), "%d", self->data.i32_val);
    } else { // TYPE_INT64
        // snprintf ile int64'ü buffer'a formatla
        // Varsayım: "%lld" format specifier snprintf tarafından destekleniyor.
        chars_written = snprintf(buffer, sizeof(buffer), "%lld", (long long)self->data.i64_val); // long long casti iyi uygulama
    }

    // snprintf başarısız olursa veya buffer taşarsa chars_written negatif veya buffer_size'dan >= olur
    // chars_written > sizeof(buffer) durumu, buffer_size'ın tam olarak null sonlandırıcı için yeterli olmadığı durumda olabilir.
    if (chars_written < 0 || chars_written >= sizeof(buffer)) {
         // Formatlama veya buffer hatası
         return (c_ampersand_result){C_AMPERSAND_ERROR_GENERIC, c_ampersand_error_with_message(C_AMPERSAND_ERROR_GENERIC, "Sayıyı stringe çevirme hatası (snprintf).").message, 0}; // Basit hata mesajı döndürelim
    }

    // Stringin boyutu (null sonlandırıcı dahil)
    size_t string_len_with_null = (size_t)chars_written + 1;

    // Sonuç string için bellek tahsis et (C& bellek API)
    char *result_string_ptr = NULL;
    c_ampersand_result alloc_res = c_ampersand_allocate(string_len_with_null, (void**)&result_string_ptr);
    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek tahsis hatası
        // result value'nun string_val alanı hala NULL olmalı (memset ile sıfırlanmıştı)
        return alloc_res; // OOM hatasını ilet
    }

    // Buffer'daki stringi tahsis edilen belleğe kopyala
    memcpy(result_string_ptr, buffer, string_len_with_null);

    // Sonuç value'yu ayarla (tip ve string değeri)
    // Varsayım: c_ampersand_type_string c&_types.h'de tanımlı bir tip objesi/struct'ıdır.
    // result->type da c_ampersand_type structı olmalı.
    result->type = c_ampersand_type_string; // Varsayım: c_ampersand_type_string geçerli bir tip değeri/struct'ıdır.
    // c_ampersand_value'nun string_val alanı struct { char* ptr; size_t size; } olarak güncellenmişti.
    result->data.string_val.ptr = result_string_ptr;
    result->data.string_val.size = chars_written; // size alanı null sonlandırıcı hariç uzunluk olabilir. Varsayıma göre 0-len kullanıyoruz.


    // Sonuç value'nun diğer alanlarını (varsa, örn. is_gc_managed) ayarla
    // String değer GC yönetimi gerektirir varsayımı.
    // result->is_gc_managed = true; // Allocate edildiği için GC yönetimi gerekli.


    return C_AMPERSAND_OK_RESULT; // Başarı
}


// --- Örnek Metot Kayıt İşlemi (başka bir yerde yapılabilir) ---

// Bu fonksiyon, sistem başlatılırken ilgili metotları kaydetmek için çağrılabilir.
c_ampersand_result register_example_methods() {
    c_ampersand_result result = C_AMPERSAND_OK_RESULT;
    c_ampersand_method *len_method = NULL;
    c_ampersand_method *to_str_method_int = NULL;
    c_ampersand_method *to_str_method_long = NULL;

    // "length" metot objesini oluştur (string tipi için)
    result = c_ampersand_method_create("length", TYPE_STRING, string_length, &len_method);
    if (result.code != C_AMPERSAND_OK) return result; // Oluşturma hatasını ilet

    // Oluşturulan metodu sisteme kaydet
    result = c_ampersand_method_register(TYPE_STRING, len_method);
    if (result.code != C_AMPERSAND_OK) {
        // Kayıt hatası, metot objesini serbest bırak
        c_ampersand_method_free(len_method); // Free hatasını kontrol etmiyoruz (kritik)
        return result; // Kayıt hatasını ilet
    }


    // "to_string" metot objesini oluştur (int32 tipi için)
    result = c_ampersand_method_create("to_string", TYPE_INT32, int_to_string, &to_str_method_int);
    if (result.code != C_AMPERSAND_OK) {
        // len_method ve len_method'un kaydı başarılı olduysa onları temizlemeye gerek yok.
        // Sadece bu create hatasını ilet.
        return result;
    }

    // Oluşturulan metodu sisteme kaydet
    result = c_ampersand_method_register(TYPE_INT32, to_str_method_int);
    if (result.code != C_AMPERSAND_OK) {
        // Kayıt hatası, metot objesini serbest bırak
        c_ampersand_method_free(to_str_method_int);
        return result;
    }

    // "to_string" metot objesini oluştur (int64 tipi için)
    result = c_ampersand_method_create("to_string", TYPE_INT64, int_to_string, &to_str_method_long);
    if (result.code != C_AMPERSAND_OK) {
         // to_str_method_int ve kaydı başarılı olduysa temizlemeye gerek yok.
         // Sadece bu create hatasını ilet.
         return result;
    }

    // Oluşturulan metodu sisteme kaydet
    result = c_ampersand_method_register(TYPE_INT64, to_str_method_long);
    if (result.code != C_AMPERSAND_OK) {
        // Kayıt hatası, metot objesini serbest bırak
        c_ampersand_method_free(to_str_method_long);
        return result;
    }


    return C_AMPERSAND_OK_RESULT; // Tüm kayıtlar başarıyla tamamlandı
}

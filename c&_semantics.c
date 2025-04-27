#include "c&_semantics.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println, c_ampersand_error_with_message için
#include "c&_stdlib.h"
// AST düğüm tipleri ve yapısı için
#include "c&_syntax.h" // c_ampersand_ast_node, AST_NODE_* enumları
// Gerçek veri tipleri tanımı ve yardımcı fonksiyonlar için
#include "c&_types.h" // c_ampersand_type tanımı, c_ampersand_type_equals, c_ampersand_type_is_compatible (varsayım)
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"

#include <string.h> // strlen, strcmp, memcpy için
#include <stdbool.h> // bool için
#include <stdarg.h> // va_list, va_start, va_end, vsnprintf için (vsnprintf Sahne/C& stdlib'de varsa)


// --- Sembol Tablosu Yönetimi ---

// Yeni bir sembol tablosu oluşturur. Heap'te c_ampersand_allocate ile tahsis edilir.
// parent: Yeni tablonun üst kapsamı.
// Dönüş: pointer veya NULL.
c_ampersand_symbol_table *c_ampersand_semantics_create_symbol_table(c_ampersand_symbol_table *parent) {
    c_ampersand_symbol_table *table = NULL;
    size_t table_struct_size = sizeof(c_ampersand_symbol_table);
    // Sembol tablosu struct'ı için bellek tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(table_struct_size, (void**)&table);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği, hata c_ampersand_allocate içinde raporlanır.
        return NULL; // NULL döndür
    }

    // Tahsis edilen belleği sıfırla
    memset(table, 0, table_struct_size);

    // Alanları ayarla
    table->head = NULL;
    table->parent = parent; // Üst kapsamı kaydet
    table->table_allocated_size = table_struct_size; // Tahsis edilen boyutu kaydet

    return table; // Başarı
}

// Bir sembol tablosunu ve içerdiği tüm sembol girdilerini rekürsif olarak serbest bırakır.
// c_ampersand_free kullanır.
c_ampersand_result c_ampersand_semantics_free_symbol_table(c_ampersand_symbol_table *table) {
    if (table == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL pointer'ı serbest bırakmak güvenlidir
    }

    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT; // Genel sonucu takip et

    // Sembol girdilerini serbest bırak (bağlı listeyi gez)
    c_ampersand_symbol_entry *current_entry = table->head;
    while (current_entry != NULL) {
        c_ampersand_symbol_entry *next_entry = current_entry->next;

        // Sembol adını serbest bırak
        if (current_entry->name != NULL && current_entry->name_allocated_size > 0) {
            c_ampersand_result res = c_ampersand_free(current_entry->name, current_entry->name_allocated_size);
            if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
            current_entry->name = NULL;
            current_entry->name_allocated_size = 0;
        }
        // ... varsa diğer heap'te ayrılmış sembol bilgileri ...

        // Sembol girdi struct'ını serbest bırak
        if (current_entry->entry_allocated_size > 0) {
            c_ampersand_result res = c_ampersand_free(current_entry, current_entry->entry_allocated_size);
            if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
        } else {
             c_ampersand_println("SEMANTICS HATA: Free edilmek istenen sembol girdisi entry_allocated_size 0.");
              if (final_result.code == C_AMPERSAND_OK) final_result = (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
        }

        current_entry = next_entry; // Sonraki girdiye geç
    }
    table->head = NULL; // Liste başını NULL yap

    // Sembol tablosu struct'ını serbest bırak
    if (table->table_allocated_size > 0) {
        c_ampersand_result res = c_ampersand_free(table, table->table_allocated_size);
        if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
    } else {
         c_ampersand_println("SEMANTICS HATA: Free edilmek istenen sembol tablosu table_allocated_size 0.");
         if (final_result.code == C_AMPERSAND_OK) final_result = (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Not: Üst kapsam tablosu burada serbest bırakılmaz. Free işlemi çağrısı yapanın sorumluluğundadır.
    // Eğer kapsamlar bir stackte tutuluyorsa, stack pop edildikçe free edilmelidir.

    return final_result; // Genel serbest bırakma sonucunu dön
}


// Bir sembolü geçerli sembol tablosuna ekler.
// Sembol zaten mevcut kapsamda tanımlıysa hata döndürür.
c_ampersand_result c_ampersand_semantics_add_symbol(c_ampersand_semantic_context *context, const char *name, c_ampersand_type type) {
    // Parametre doğrulama
    if (context == NULL || context->current_scope == NULL || name == NULL || strlen(name) == 0) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    // Sembolün mevcut kapsamda zaten tanımlı olup olmadığını kontrol et
    // Sadece mevcut kapsamı kontrol etmek için ayrı bir yardımcı fonk. yazılabilir veya lookup_symbol'ın bir varyantı.
    // Şimdilik basit bir döngü ile kontrol edelim.
    c_ampersand_symbol_entry *current_entry_check = context->current_scope->head;
    while (current_entry_check != NULL) {
        if (current_entry_check->name != NULL && strcmp(current_entry_check->name, name) == 0) {
            // Sembol zaten mevcut kapsamda tanımlı! Semantik hata.
            // Konum bilgisi context veya AST düğümünden alınmalı.
            // Bu fonksiyon AST düğümünü almadığı için burada konum bilgisi yok.
            // Hata raporlama analyze_* fonksiyonlarında yapılmalı.
            // Burada sadece hata kodunu dönelim.
            return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, c_ampersand_error_with_message(C_AMPERSAND_ERROR_SEMANTIC, "Sembol '%s' mevcut kapsamda zaten tanımlı.", name).message, 0};
        }
        current_entry_check = current_entry_check->next;
    }


    // Yeni sembol girdisi için bellek tahsis et
    c_ampersand_symbol_entry *new_entry = NULL;
    size_t entry_struct_size = sizeof(c_ampersand_symbol_entry);
    c_ampersand_result alloc_res = c_ampersand_allocate(entry_struct_size, (void**)&new_entry);

    if (alloc_res.code != C_AMPERSAND_OK) {
        return alloc_res; // Bellek hatasını ilet
    }

    // Belleği sıfırla
    memset(new_entry, 0, entry_struct_size);

    // Sembol adı stringi için bellek tahsis et ve kopyala
    size_t name_len = strlen(name);
    size_t name_alloc_size = name_len + 1; // Null sonlandırıcı dahil
    char *name_ptr = NULL;
    alloc_res = c_ampersand_allocate(name_alloc_size, (void**)&name_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Ad stringi tahsis hatası, girdi struct'ını serbest bırak
        c_ampersand_free(new_entry, entry_struct_size);
        return alloc_res; // Bellek hatasını ilet
    }

    // Ad stringini kopyala
    memcpy(name_ptr, name, name_alloc_size);


    // Sembol girdisini doldur
    new_entry->name = name_ptr;
    new_entry->name_allocated_size = name_alloc_size; // Ad stringi boyutu
    new_entry->type = type; // Tip structı kopyalanır
    new_entry->entry_allocated_size = entry_struct_size; // Girdi structı boyutu
    new_entry->next = context->current_scope->head; // Mevcut listeye ekle (başına)


    // Sembol girdisini sembol tablosuna ekle
    context->current_scope->head = new_entry;

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir sembolü geçerli kapsamdan başlayarak üst kapsamlı sembol tablolarında arar.
// context: Semantik bağlam.
// name: Aranacak sembol adı.
// Dönüş: Bulunan sembol girdisinin pointer'ı veya bulunamazsa NULL.
c_ampersand_symbol_entry *c_ampersand_semantics_lookup_symbol(c_ampersand_semantic_context *context, const char *name) {
    // Parametre doğrulama
    if (context == NULL || name == NULL || strlen(name) == 0) {
        return NULL;
    }

    // Mevcut kapsamdan başlayarak üst kapsamlara doğru ara
    c_ampersand_symbol_table *current_table = context->current_scope;
    while (current_table != NULL) {
        // Mevcut sembol tablosunun girdilerini gez
        c_ampersand_symbol_entry *current_entry = current_table->head;
        while (current_entry != NULL) {
            // Sembol adlarını karşılaştır
            if (current_entry->name != NULL && strcmp(current_entry->name, name) == 0) {
                return current_entry; // Sembol bulundu
            }
            current_entry = current_entry->next; // Sonraki girdiye geç
        }
        // Mevcut kapsamda bulunamadı, üst kapsama geç
        current_table = current_table->parent; // Üst kapsam bağlantısını kullan
    }

    // Sembol hiçbir kapsamda bulunamadı
    return NULL;
}

// İki tipin uyumlu olup olmadığını kontrol eder (örn. atama veya ikili operasyonlar için).
// Bu, gerçek tip sistemine ve dilin kurallarına göre implemente edilmelidir.
// Basit == kontrolü yerine daha karmaşık mantık gereklidir.
bool c_ampersand_semantics_check_type_compatibility(c_ampersand_type type1, c_ampersand_type type2) {
    // Varsayım: c&_types.h'de c_ampersand_type_equals ve c_ampersand_type_is_compatible fonksiyonları var.
     c_ampersand_type_equals tam tip eşitliği kontrolü yapar (tüm alanlar dahil).
     c_ampersand_type_is_compatible atama/operasyon uyumluluğunu kontrol eder (örn. int -> float).

    // Basit örnek: Tam tip eşitliği veya temel sayısal dönüşümlerin uyumlu olduğu varsayımı.
    if (c_ampersand_type_equals(type1, type2)) {
        return true; // Tipler tamamen aynı
    }

    // Örnek: int32 -> float64 gibi temel sayısal genişletmelerin uyumlu olduğunu varsayalım.
    // Gerçek implementasyonda bu, type struct'larının içeriğine ve bir kural setine bakmalıdır.
     if (type1.kind == TYPE_INT32 && type2.kind == TYPE_FLOAT64) return true;
     if (type1.kind == TYPE_INT64 && type2.kind == TYPE_FLOAT64) return true;
    // ... diğer dönüşümler

    // Daha gelişmiş bir yaklaşım:
    // return c_ampersand_type_is_compatible(type1, type2); // c&_types.c'den gelen fonksiyon

    c_ampersand_println("SEMANTICS NOT: check_type_compatibility sadece tam eşitliği kontrol ediyor (placeholder).");
    // Placeholder olarak sadece tam eşitliği kontrol edelim veya her zaman false dönelim.
    // Tam eşitlik kontrolü:
    // return c_ampersand_type_equals(type1, type2);
    // Veya basitlik adına sadece kind karşılaştırması (type struct değil enum gibi davranırsa):
     return type1.kind == type2.kind; // Veya type1 == type2 eğer type structları doğrudan karşılaştırılabilirse.
}

// Semantik bir hata bildirir. Hata mesajını formatlar ve raporlar (c_ampersand_println kullanarak).
// node: Hatanın meydana geldiği AST düğümü (konum bilgisi için). NULL olabilir.
// message: Format stringi.
// ...: Format stringi için argümanlar.
// Dönüş: Hata durumunda uygun bir c_ampersand_result (şu an için C_AMPERSAND_ERROR_SEMANTIC).
// NOT: Bu fonksiyon hataları basar ancak analizin devam etmesini sağlamak için C_AMPERSAND_OK_RESULT dönebilir.
// analyze_* fonksiyonları hatanın yayılması gerekiyorsa kendileri uygun hata koduyla result döndürmelidir.
c_ampersand_result c_ampersand_semantics_report_error(c_ampersand_ast_node *node, const char *message, ...) {
    // Hata mesajını formatlamak için bir buffer kullanalım.
    // vsnprintf'nin Sahne/C& stdlib'de mevcut olduğunu varsayalım.
    char formatted_message[512]; // Sabit boyutlu buffer (daha sağlam implementasyon dinamik buffer kullanabilir)

    // Hata mesajının başını oluştur
    int offset = snprintf(formatted_message, sizeof(formatted_message), "Semantik Hata");

    // Konum bilgisi ekle (AST düğümü ve token varsa)
    if (node != NULL && node->token != NULL) {
        offset += snprintf(formatted_message + offset, sizeof(formatted_message) - offset, " (Satır: %d, Sütun: %d)", node->token->line, node->token->column);
    }

    // Mesajın geri kalanını formatla
    offset += snprintf(formatted_message + offset, sizeof(formatted_message) - offset, ": ");

    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message + offset, sizeof(formatted_message) - offset, message, args);
    va_end(args);

    // Formatlanmış mesajı c_ampersand_println ile yazdır (stderr yerine)
    c_ampersand_println("%s", formatted_message); // c_ampersand_println stderr'a yazıyor varsayalım

    // Semantik hatalar genellikle analizin devam etmesine izin verir ancak sonuçta bir hata bayrağı set edilmelidir.
    // Bu fonksiyonun sadece raporlama yapıp C_AMPERSAND_OK_RESULT dönmesi ve analyze_* fonksiyonlarının
    // analiz başarısız olursa (çok fazla hata vb.) C_AMPERSAND_ERROR_SEMANTIC dönmesi daha yaygındır.
    // Mevcut yapıya uyum sağlamak için, burada C_AMPERSAND_ERROR_SEMANTIC koduyla bir result döndürelim,
    // ancak mesajı zaten yazdırdık.
    // Alternatif olarak, bağlam içinde bir hata listesi tutulabilir.

    return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0}; // Semantik hata kodu dön
}


// AST düğümlerini analiz eden yardımcı fonksiyon (rekürsif)
static c_ampersand_result analyze_node(c_ampersand_ast_node *node, c_ampersand_semantic_context *context);

// Program düğümünü analiz eder (genellikle kök).
static c_ampersand_result analyze_program(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
     if (node == NULL || context == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
     }
     // Program seviyesi için ek kapsam yönetimi gerekmez, global kapsam kullanılır.
    for (size_t i = 0; i < node->num_children; ++i) {
        // Her çocuk düğümü analiz et
        c_ampersand_result result = analyze_node(node->children[i], context);
        if (result.code != C_AMPERSAND_OK) {
            return result; // Hata durumunda analizi durdur ve hatayı yay
        }
    }
    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Değişken bildirimi düğümünü analiz eder.
static c_ampersand_result analyze_variable_declaration(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
     // Parametre doğrulama
     if (node == NULL || context == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
     }
     // Düğüm yapısını kontrol et (örn. ID ve Tip düğümü olmalı)
    if (node->num_children < 1 || node->children[0]->type != AST_NODE_IDENTIFIER /* || Tip düğümü eksik */) {
        c_ampersand_semantics_report_error(node, "Geçersiz değişken bildirimi yapısı."); // Yapısal semantik hata raporla
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0}; // Hata dön
    }

     // Değişken adını al
    c_ampersand_token *identifier_token = node->children[0]->token;
     if (identifier_token == NULL || identifier_token->lexeme == NULL) {
         c_ampersand_semantics_report_error(node, "Geçersiz değişken adı belirteci.");
         return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0};
     }

     // Değişkenin tipini AST'den al (varsayım: AST yapısı tip bilgisini tutuyor)
     // Örneğin: node->children[1] tip düğümü olabilir veya tip doğrudan node'un bir alanında saklı olabilir.
     c_ampersand_type variable_type;
     // --- PLACEHOLDER: Gerçek tip bilgisi AST'den alınmalıdır. ---
      if (node->num_children > 1 && node->children[1]->type == AST_NODE_TYPE) {
         variable_type = get_type_from_ast_node(node->children[1]); // Varsayımsal yardımcı fonksiyon
      } else {
         c_ampersand_semantics_report_error(node, "Değişken bildirimi için tip bilgisi eksik.");
         return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0};
      }
     // Basitlik adına placeholder olarak varsayılan bir tip kullanalım (TYPE_INT32 c&_types.h'den)
     variable_type = (c_ampersand_type){TYPE_INT32, /* diğer type alanları ... */};
     c_ampersand_println("SEMANTICS NOT: Değişken tipi AST'den alınacak: %s -> INT32 (Placeholder)", identifier_token->lexeme);
     // --- PLACEHOLDER SONU ---


     // Sembol tablosuna ekle
    c_ampersand_result add_res = c_ampersand_semantics_add_symbol(context, identifier_token->lexeme, variable_type);

     if (add_res.code != C_AMPERSAND_OK) {
         // Sembol ekleme hatası (örn. zaten tanımlı veya bellek hatası)
         // add_symbol zaten ilgili hata kodunu ve mesajı döner.
         // Eğer add_symbol mesajlı hata dönüyorsa, onu olduğu gibi yayalım.
         // Eğer add_symbol sadece kod dönüyorsa, burada mesajlı sarmalayalım.
         // add_symbol artık mesajlı hata dönebiliyor.
         if (add_res.code == C_AMPERSAND_ERROR_SEMANTIC) { // Sembol zaten tanımlı hatası
              c_ampersand_semantics_report_error(node, add_res.message); // Hatayı konumuyla raporla
               c_ampersand_result_free(&add_res); // add_symbol allocate ettiyse mesajı serbest bırak
         } else { // Diğer hatalar (bellek vb.)
              c_ampersand_semantics_report_error(node, "Sembol eklenirken hata: %s", add_res.message);
                c_ampersand_result_free(&add_res);
         }
         return (c_ampersand_result){add_res.code, NULL, 0}; // Hata kodunu yay, mesajı zaten raporladık
     }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// İkili ifade düğümünü analiz eder. Operand tiplerini kontrol eder, sonuç tipini belirler.
static c_ampersand_result analyze_binary_expression(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    // Parametre doğrulama
    if (node == NULL || context == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Düğüm yapısını kontrol et (örn. 2 çocuk operand olmalı)
    if (node->num_children != 2) {
        c_ampersand_semantics_report_error(node, "Geçersiz ikili ifade yapısı.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0};
    }

    // Sol ve sağ operand düğümlerini analiz et (rekürsif).
    // Bu analizler operand düğümlerinin tiplerini belirlemeli (veya çıkarmalı) ve AST düğümünde saklamalıdır.
    c_ampersand_result left_res = analyze_node(node->children[0], context);
    if (left_res.code != C_AMPERSAND_OK) return left_res; // Sol operand analiz hatası

    c_ampersand_result right_res = analyze_node(node->children[1], context);
    if (right_res.code != C_AMPERSAND_OK) return right_res; // Sağ operand analiz hatası

    // Sol ve sağ operandların tiplerini AST düğümlerinden al (varsayım: analyze_node bunları belirler)
     c_ampersand_type left_type = node->children[0]->semantic_type; // Varsayımsal semantic_type alanı AST düğümünde
     c_ampersand_type right_type = node->children[1]->semantic_type; // Varsayımsal semantic_type alanı AST düğümünde

    // --- PLACEHOLDER: Operand tipleri AST'den alınacak ve operatörle uyumluluğu kontrol edilecek. ---
    c_ampersand_type left_type = {TYPE_UNKNOWN, 0}; // Placeholder
    c_ampersand_type right_type = {TYPE_UNKNOWN, 0}; // Placeholder
    // Gerçek implementasyonda operand düğümlerinin tipleri burada elde edilmeli.
    c_ampersand_println("SEMANTICS NOT: İkili ifade operand tipleri AST'den alınacak.");
    // --- PLACEHOLDER SONU ---


    // Operatör tipini al (varsayım: operatör düğümün bir alanında saklı)
     c_ampersand_operator_type op_type = node->operator_type; // Varsayımsal operator_type alanı

    // --- PLACEHOLDER: Operatör tipine göre tip kontrolü ve sonuç tipi çıkarımı. ---
    // Örnek operatör tipi: OPERATOR_ADD (c&_operators.h'den)
     if (!c_ampersand_semantics_check_type_compatibility(left_type, right_type)) {
         // Tipler uyumsuz, hata raporla
         c_ampersand_semantics_report_error(node, "İkili ifade için uyumsuz operand tipleri.");
         return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH, NULL, 0};
     }
    // Sonuç tipini çıkar (örn. int + float -> float). Bu da karmaşık olabilir.
     c_ampersand_type result_type = infer_result_type(left_type, right_type, op_type); // Varsayımsal fonksiyon
     node->semantic_type = result_type; // Sonuç tipini AST düğümünde sakla
     c_ampersand_println("SEMANTICS NOT: İkili ifade tip kontrolü ve çıkarımı implemente edilecek.");
    // --- PLACEHOLDER SONU ---

    return C_AMPERSAND_OK_RESULT; // Başarı (şimdilik tip kontrolü yapılmıyor)
}

// Tanımlayıcı (Identifier) düğümünü analiz eder. Sembol tablosunda arama yapar.
static c_ampersand_result analyze_identifier(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    // Parametre doğrulama
    if (node == NULL || context == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // Token ve lexeme var mı kontrol et
    if (node->token == NULL || node->token->lexeme == NULL) {
        c_ampersand_semantics_report_error(node, "Geçersiz tanımlayıcı belirteci.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0};
    }

    // Sembol tablosunda arama yap (mevcut ve üst kapsamlarda)
    c_ampersand_symbol_entry *symbol = c_ampersand_semantics_lookup_symbol(context, node->token->lexeme);

    if (symbol == NULL) {
        // Sembol bulunamadı, tanımlanmamış değişken hatası raporla
        c_ampersand_semantics_report_error(node, "Tanımlanmamış sembol '%s'.", node->token->lexeme);
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC, NULL, 0}; // Hata dön
    }

    // Sembol bulundu, semantik analiz geçti.
    // İsteğe bağlı: Sembol girdisine bir pointer'ı AST düğümünde sakla
     node->symbol_entry = symbol; // Varsayımsal alan

    // İsteğe bağlı: Tanımlayıcının tipini AST düğümünde sakla (lookup sonucu)
     node->semantic_type = symbol->type; // Varsayımsal alan

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// AST düğüm tiplerine göre uygun analiz fonksiyonunu çağıran dispatch fonksiyonu.
static c_ampersand_result analyze_node(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    if (node == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL düğümler için başarı dön
    }
    // context NULL olmamalı, analyze_* fonksiyonları bunu kontrol ediyor.

    c_ampersand_result result = C_AMPERSAND_OK_RESULT;

    switch (node->type) {
        case AST_NODE_PROGRAM:
            result = analyze_program(node, context);
            break;
        case AST_NODE_VARIABLE_DECLARATION:
            result = analyze_variable_declaration(node, context);
            break;
        case AST_NODE_BINARY_EXPRESSION:
            result = analyze_binary_expression(node, context);
            break;
        case AST_NODE_IDENTIFIER:
            result = analyze_identifier(node, context);
            break;
        // --- PLACEHOLDER: Diğer AST düğüm tipleri için analiz fonksiyonları ---
         case AST_NODE_FUNCTION_DECLARATION:
        //     // Yeni kapsam oluştur, parametreleri sembol tablosuna ekle, fonksiyon gövdesini analiz et, kapsamdan çık, sembol tablosunu free et.
             break;
         case AST_NODE_BLOCK:
        //     // Yeni kapsam oluştur, block içindeki ifadeleri analiz et, kapsamdan çık, sembol tablosunu free et.
             break;
         case AST_NODE_LITERAL:
        //     // Literal tipini belirle ve AST düğümünde sakla.
             break;
         case AST_NODE_ASSIGNMENT:
        //     // Sağ ve sol tarafları analiz et, tiplerini kontrol et, atama uyumluluğunu kontrol et.
             break;
         case AST_NODE_FUNCTION_CALL:
        //     // Fonksiyon sembolünü lookup yap, argüman tiplerini analiz et ve parametre tipleriyle uyumluluğunu kontrol et.
             break;
        // ... diğer tüm düğüm tipleri ...
        // --- PLACEHOLDER SONU ---

        default:
            // Bilinmeyen veya henüz ele alınmamış düğüm tipi
            c_ampersand_println("SEMANTICS NOT: Analiz edilmeyen AST düğüm tipi: %d", node->type);
            break; // Analize devam et
    }

    // Semantik analiz genellikle tüm hataları toplar ve sonunda raporlar.
    // analyze_* fonksiyonları C_AMPERSAND_OK_RESULT dönebilir ve hataları bağlam içinde saklayabilir.
    // Veya analyze_* fonksiyonları ilk kritik hatada hemen hata döndürebilir.
    // Mevcut implementasyon ilk analyze_* hatasında hemen dönüyor.

    return result; // Analiz sonucunu yay
}

// Semantik analiz sürecini başlatır. Global sembol tablosu oluşturur ve AST'yi gezer.
c_ampersand_result c_ampersand_semantics_analyze(c_ampersand_ast_node *root) {
     // Parametre doğrulama
     if (root == NULL) {
         c_ampersand_println("SEMANTICS HATA: Analiz edilecek kök AST düğümü NULL.");
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
     }

    c_ampersand_println("SEMANTICS BILGI: Semantik analiz başlatılıyor...");

    // Global kapsam sembol tablosunu oluştur
    c_ampersand_symbol_table *global_scope = c_ampersand_semantics_create_symbol_table(NULL); // Global kapsamın üst kapsamı yok
    if (global_scope == NULL) {
        // create_symbol_table zaten bellek hatasını raporlar
        c_ampersand_println("SEMANTICS HATA: Global sembol tablosu oluşturulamadı (bellek yetersiz?).");
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0};
    }

    // Semantik bağlamı oluştur ve global kapsamı ayarla
    c_ampersand_semantic_context context;
    memset(&context, 0, sizeof(c_ampersand_semantic_context)); // Bağlamı sıfırla
    context.current_scope = global_scope;
    // ... bağlamdaki diğer alanları başlat (hata listesi vb.)

    // AST üzerinde semantik analizi başlat (analyze_node rekürsif çağrıları tetikler)
    c_ampersand_result result = analyze_node(root, &context);

    // Not: Semantik analiz sırasında bulunan tüm hatalar genellikle bağlam içinde toplanır
    // ve analiz bitince toplam hata sayısı kontrol edilir.
    // Eğer context içinde hata listesi olsaydı, burada kontrol edilirdi.
     if (context.error_count > 0) {
         result.code = C_AMPERSAND_ERROR_SEMANTIC;
    //     // result.message = ... toplam hata sayısı mesajı ...
     }

    // Global sembol tablosunu ve tüm içeriğini serbest bırak
    // Bu, iç içe geçmiş tabloları da serbest bırakmalıdır (analyze_* fonksiyonları kapsamdan çıkarken alt tabloları free etmeli).
    // Veya free_symbol_table recursive olmalı, ancak scope yapısını bilmeden recursive free riskli.
    // En güvenlisi: analyze_node fonksiyonları kapsam giriş/çıkışını yönetirken alt kapsamları kendileri free etmeli.
    // Şimdilik sadece global kapsam tablosunu free edelim (içindeki semboller free_symbol_table tarafından free edilir).
    c_ampersand_result free_res = c_ampersand_semantics_free_symbol_table(global_scope);
    if (free_res.code != C_AMPERSAND_OK) {
         c_ampersand_println("SEMANTICS UYARI: Global sembol tablosu serbest bırakılırken hata.");
          c_ampersand_result_print_error(free_res);
          c_ampersand_result_free(&free_res);
         // Bellek serbest bırakma hatası, analizin genel sonucunu etkilemeyebilir veya etkileyebilir.
         // Şimdilik sadece uyarı verelim.
    }


    c_ampersand_println("SEMANTICS BILGI: Semantik analiz tamamlandı.");

    // Analizin genel sonucunu dön (analyze_node'dan gelen veya toplanan hatalara göre belirlenen)
    return result;
}

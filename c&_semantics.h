#ifndef C_AMPERSAND_SEMANTICS_H
#define C_AMPERSAND_SEMANTICS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// AST düğüm tipleri ve yapısı için
#include "c&_syntax.h" // c_ampersand_ast_node, AST_NODE_* enumları
// Gerçek veri tipleri tanımı için (sadece enum değil struct olmalı)
#include "c&_types.h" // c_ampersand_type tanımı buradan gelmelidir.

#include <stdbool.h> // bool için
#include <stddef.h> // size_t için

// --- Veri Tipleri ---
 c_ampersand_type tanımı artık c&_types.h'den gelmelidir.
 typedef enum { ... } c_ampersand_type; // BU TANIM KALDIRILDI


// Bir sembol tablosu girdisini temsil eder. Heap üzerinde yaşar.
typedef struct c_ampersand_symbol_entry {
    char *name; // Sembol adı (heap'te ayrılmış)
    size_t name_allocated_size; // name stringi için ayrılan boyut (free için)
    c_ampersand_type type; // Sembolün tipi (c_ampersand_type structı olmalı)
    // ... diğer sembol bilgileri (konum, depolama sınıfı, değeri, vb.) eklenebilir

    // Bu sembol girişi struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t entry_allocated_size;

    struct c_ampersand_symbol_entry *next; // Aynı kapsamdaki bir sonraki girdi için
} c_ampersand_symbol_entry;

// Sembol tablosunu temsil eder (bir kapsam - scope için). Heap üzerinde yaşar.
typedef struct c_ampersand_symbol_table {
    c_ampersand_symbol_entry *head; // Kapsamdaki sembol listesinin başı
    struct c_ampersand_symbol_table *parent; // Üst kapsam (daha dıştaki scope)

    // Bu sembol tablosu struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t table_allocated_size;

    // ... diğer kapsam bilgileri eklenebilir (örn. kapsam adı, derinlik)
} c_ampersand_symbol_table;

// Semantik analiz bağlamını tutan yapı
typedef struct c_ampersand_semantic_context {
    // Mevcut kapsamın sembol tablosuna işaretçi
    c_ampersand_symbol_table *current_scope;
    // ... diğer bağlam bilgileri (hata listesi, global sembol tablosu pointer'ı, vb.) eklenebilir

    // Hata raporlama için buffer ve pozisyon
     char error_message_buffer[512]; // veya dinamik
     size_t error_message_buffer_pos;
     int error_count; // Toplanan hata sayısı
} c_ampersand_semantic_context;


// Semantik analiz sürecini başlatır. AST kök düğümünü alır.
// root: Analiz edilecek AST'nin kök düğümü.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Analiz sırasında bulunan semantik hatalar raporlanır (println veya context içinde saklanır).
c_ampersand_result c_ampersand_semantics_analyze(c_ampersand_ast_node *root);


// Yeni bir sembol tablosu oluşturur. Heap'te tahsis edilir.
// parent: Yeni tablonun üst kapsamı olacak sembol tablosu (NULL global kapsam için).
// Dönüş: Oluşturulan sembol tablosunun pointer'ı veya bellek yetersizliği durumunda NULL.
c_ampersand_symbol_table *c_ampersand_semantics_create_symbol_table(c_ampersand_symbol_table *parent);

// Bir sembol tablosunu ve içerdiği tüm sembol girdilerini rekürsif olarak serbest bırakır.
// table: Serbest bırakılacak sembol tablosu.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek serbest bırakma hatalarını raporlayabilir.
c_ampersand_result c_ampersand_semantics_free_symbol_table(c_ampersand_symbol_table *table);


// Bir sembolü geçerli sembol tablosuna ekler.
// context: Semantik bağlam (mevcut kapsamı içerir).
// name: Eklenecek sembol adı.
// type: Eklenecek sembolün tipi.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz argüman, bellek yetersizliği, sembol zaten tanımlı gibi hataları raporlar.
c_ampersand_result c_ampersand_semantics_add_symbol(c_ampersand_semantic_context *context, const char *name, c_ampersand_type type);

// Bir sembolü geçerli kapsamdan başlayarak üst kapsamlı sembol tablolarında arar.
// context: Semantik bağlam (mevcut kapsamı içerir).
// name: Aranacak sembol adı.
// Dönüş: Bulunan sembol girdisinin pointer'ı veya bulunamazsa NULL.
c_ampersand_symbol_entry *c_ampersand_semantics_lookup_symbol(c_ampersand_semantic_context *context, const char *name);


// İki tipin uyumlu olup olmadığını kontrol eder (örn. atama veya ikili operasyonlar için).
// type1: Birinci tip.
// type2: İkinci tip.
// Dönüş: Uyumlu ise true, değilse false.
bool c_ampersand_semantics_check_type_compatibility(c_ampersand_type type1, c_ampersand_type type2);


// Semantik bir hata bildirir. Hata mesajını formatlar ve raporlar.
// node: Hatanın meydana geldiği AST düğümü (konum bilgisi için).
// message: Format stringi.
// ...: Format stringi için argümanlar.
// Dönüş: Hata durumunda uygun bir c_ampersand_result (örn. C_AMPERSAND_ERROR_SEMANTIC).
c_ampersand_result c_ampersand_semantics_report_error(c_ampersand_ast_node *node, const char *message, ...);


// --- Kapsam Yönetimi Yardımcı Fonksiyonları (Bağlam içinde yönetilebilir) ---
 c_ampersand_result c_ampersand_semantics_enter_scope(c_ampersand_semantic_context *context, c_ampersand_symbol_table *new_scope);
 c_ampersand_result c_ampersand_semantics_exit_scope(c_ampersand_semantic_context *context);


#endif // C_AMPERSAND_SEMANTICS_H

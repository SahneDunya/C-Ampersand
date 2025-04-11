#ifndef C_AMPERSAND_SEMANTICS_H
#define C_AMPERSAND_SEMANTICS_H

#include "c&_stdlib.h" // Temel tipler için
#include "c&_syntax.h" // AST düğüm tipleri için
#include <stdbool.h>

// --- Veri Tipleri ---
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    // ... diğer tipler eklenebilir
    TYPE_COUNT
} c_ampersand_type;

// Bir sembol tablosu girdisini temsil eder.
typedef struct c_ampersand_symbol_entry {
    char *name;
    c_ampersand_type type;
    // ... diğer sembol bilgileri (scope, değer vb.) eklenebilir
    struct c_ampersand_symbol_entry *next; // Zincirleme için
} c_ampersand_symbol_entry;

// Sembol tablosunu temsil eder.
typedef struct {
    c_ampersand_symbol_entry *head;
} c_ampersand_symbol_table;

// Semantik analiz bağlamını tutan yapı (isteğe bağlı)
typedef struct c_ampersand_semantic_context {
    c_ampersand_symbol_table *current_scope;
    // ... diğer bağlam bilgileri (hata listesi vb.) eklenebilir
} c_ampersand_semantic_context;

// Semantik analiz sürecini başlatır.
c_ampersand_result c_ampersand_semantics_analyze(c_ampersand_ast_node *root);

// Yeni bir sembol tablosu oluşturur.
c_ampersand_symbol_table *c_ampersand_semantics_create_symbol_table(c_ampersand_symbol_table *parent);

// Bir sembolü geçerli sembol tablosuna ekler.
c_ampersand_result c_ampersand_semantics_add_symbol(c_ampersand_semantic_context *context, const char *name, c_ampersand_type type);

// Bir sembolü geçerli ve üst kapsamlı sembol tablolarında arar.
c_ampersand_symbol_entry *c_ampersand_semantics_lookup_symbol(c_ampersand_semantic_context *context, const char *name);

// İki tipin uyumlu olup olmadığını kontrol eder.
bool c_ampersand_semantics_check_type_compatibility(c_ampersand_type type1, c_ampersand_type type2);

// Semantik bir hata bildirir.
void c_ampersand_semantics_report_error(c_ampersand_ast_node *node, const char *message, ...);

// ... diğer semantik analiz ile ilgili fonksiyonlar eklenebilir
// (örneğin, tip çıkarımı, kapsam yönetimi)

#endif // C_AMPERSAND_SEMANTICS_H
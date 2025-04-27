#ifndef C_AMPERSAND_SYNTAX_H
#define C_AMPERSAND_SYNTAX_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// C& bellek yönetimi fonksiyonları için (c_ampersand_allocate, c_ampersand_free vb.)
#include "c&_memory.h"

#include <stdbool.h> // bool için
#include <stddef.h> // size_t için


// --- Token Tipleri ---
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOLEAN_LITERAL, // true, false

    // Anahtar Kelimeler
    TOKEN_KEYWORD_FUNC,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,

    // Operatörler
    TOKEN_OPERATOR_PLUS,        // +
    TOKEN_OPERATOR_MINUS,       // -
    TOKEN_OPERATOR_MULTIPLY,    // *
    TOKEN_OPERATOR_DIVIDE,      // /
    TOKEN_OPERATOR_ASSIGN,      // =
    TOKEN_OPERATOR_EQUAL,       // ==
    TOKEN_OPERATOR_NOT_EQUAL,   // !=
    TOKEN_OPERATOR_LESS_THAN,   // <
    TOKEN_OPERATOR_GREATER_THAN,// >
    TOKEN_OPERATOR_LESS_EQUAL,  // <=
    TOKEN_OPERATOR_GREATER_EQUAL, // >=
    TOKEN_OPERATOR_AND,         // &&
    TOKEN_OPERATOR_OR,          // ||
    TOKEN_OPERATOR_NOT,         // !

    // Ayraçlar ve Noktalama İşaretleri
    TOKEN_LEFT_PAREN,         // (
    TOKEN_RIGHT_PAREN,        // )
    TOKEN_LEFT_BRACE,         // {
    TOKEN_RIGHT_BRACE,        // }
    TOKEN_LEFT_BRACKET,       // [
    TOKEN_RIGHT_BRACKET,      // ]
    TOKEN_COMMA,              // ,
    TOKEN_SEMICOLON,          // ;
    TOKEN_COLON,              // :
    TOKEN_ARROW,              // ->
    TOKEN_DOT,                // .

    // ... diğer token tipleri eklenebilir (yorumlar, tip adları, vb.)
    TOKEN_COUNT // Tüm token tiplerinin sayısını belirtmek için (kolaylık)
} c_ampersand_token_type;

// Bir token'ı temsil eden yapı. Heap üzerinde yaşar.
typedef struct c_ampersand_token {
    c_ampersand_token_type type; // Token'ın türü
    char *lexeme;       // Token'ın metin karşılığı (heap'te ayrılmış)
    size_t lexeme_allocated_size; // lexeme stringi için ayrılan boyut (free için)
    int line;           // Token'ın bulunduğu satır
    int column;         // Token'ın bulunduğu sütun
    // Bu token struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t token_allocated_size;

    // ... diğer token bilgileri (literal değerin sayısal/bool karşılığı vb.) eklenebilir
} c_ampersand_token;

// --- Soyut Sözdizimi Ağacı (AST) Düğümleri ---

// Temel AST düğüm tipi
typedef enum {
    AST_NODE_PROGRAM,
    AST_NODE_FUNCTION_DECLARATION,
    AST_NODE_PARAMETER_DECLARATION, // Fonksiyon parametresi
    AST_NODE_VARIABLE_DECLARATION,
    AST_NODE_ASSIGNMENT_EXPRESSION,
    AST_NODE_BINARY_EXPRESSION,
    AST_NODE_UNARY_EXPRESSION,
    AST_NODE_LITERAL, // Sayısal, string, bool literal
    AST_NODE_IDENTIFIER, // Değişken, fonksiyon adı vb.
    AST_NODE_BLOCK_STATEMENT, // { ... } bloğu
    AST_NODE_IF_STATEMENT,
    AST_NODE_WHILE_STATEMENT,
    AST_NODE_RETURN_STATEMENT,
    AST_NODE_CALL_EXPRESSION, // Fonksiyon çağrısı
    AST_NODE_ARGUMENT_LIST, // Fonksiyon çağrısındaki argüman listesi
    AST_NODE_PARAMETER_LIST, // Fonksiyon bildirimindeki parametre listesi
    AST_NODE_TYPE_SPECIFIER, // Tip belirtimi (örn. int, float)

    // ... diğer AST düğüm tipleri eklenebilir (döngüler, break/continue, struct/class tanımı, üye erişimi, dizi erişimi vb.)
    AST_NODE_COUNT
} c_ampersand_ast_node_type;

// Temel AST düğümü yapısı. Heap üzerinde yaşar.
typedef struct c_ampersand_ast_node {
    c_ampersand_ast_node_type type; // Düğümün türü
    // Bu düğüm struct'ı için c_ampersand_allocate ile ayrılan boyut.
    size_t node_allocated_size;

    // ... düğüme özgü veriler (örneğin, ikili ifadeler için operatör token/tipi, literal düğümler için değer, vb.)
    // Bu alanlar union içinde veya spesifik düğüm tipleri için ayrı structlarda olabilir.
    // Örnek: union { c_ampersand_token_type operator_type; c_ampersand_value literal_value; } data;

    struct c_ampersand_ast_node **children; // Alt düğümler dizisi (heap'te ayrılmış dizi)
    size_t num_children; // Dizideki geçerli alt düğüm sayısı
    size_t children_capacity; // Dizinin mevcut kapasitesi
    size_t children_allocated_size; // children dizisi için ayrılan byte boyutu (free için)

    c_ampersand_token *token; // Bu düğüme karşılık gelen temel token (isteğe bağlı, NULL olabilir)
    // Token'ın belleği ayrı yönetilir, bu sadece bir pointer'dır.

    // Semantik analiz sırasında eklenecek bilgiler (örneğin, tür çıkarımı sonucu, sembol tablosu referansı)
     c_ampersand_type semantic_type;
     c_ampersand_symbol_entry *symbol_entry;

} c_ampersand_ast_node;


// Yeni bir token oluşturur. Lexeme stringini C& belleğinde tahsis eder.
// type: Oluşturulacak token türü.
// lexeme: Token'ın metinsel karşılığı (null sonlandırıcı olmalı).
// line, column: Token'ın kaynak koddaki konumu.
// Dönüş: Oluşturulan token pointer'ı veya bellek yetersizliği durumunda NULL.
c_ampersand_token *c_ampersand_syntax_create_token(c_ampersand_token_type type, const char *lexeme, int line, int column);

// Yeni bir AST düğümü oluşturur. Düğüm struct'ını C& belleğinde tahsis eder.
// type: Oluşturulacak AST düğüm türü.
// token: Bu düğümle ilişkili temel token (isteğe bağlı, NULL olabilir). Token belleği ayrı yönetilir.
// Dönüş: Oluşturulan AST düğümü pointer'ı veya bellek yetersizliği durumunda NULL.
c_ampersand_ast_node *c_ampersand_syntax_create_ast_node(c_ampersand_ast_node_type type, c_ampersand_token *token);

// Bir AST düğümüne alt düğüm ekler. Alt düğüm dizisini dinamik olarak genişletebilir (c_ampersand_reallocate kullanarak).
// parent: Alt düğümün ekleneceği üst düğüm.
// child: Eklenecek alt düğüm.
// Dönüş: Başarı veya hata (geçersiz argüman, bellek yetersizliği).
c_ampersand_result c_ampersand_syntax_add_child(c_ampersand_ast_node *parent, c_ampersand_ast_node *child);

// Bir AST düğümünü ve altındaki tüm düğümleri rekürsif olarak serbest bırakır.
// Token belleklerini de serbest bırakır. c_ampersand_free kullanır.
void c_ampersand_syntax_free_ast_node(c_ampersand_ast_node *node);

// Bir token'ı serbest bırakır. Lexeme stringini ve token struct'ını serbest bırakır.
// c_ampersand_free kullanır.
void c_ampersand_syntax_free_token(c_ampersand_token *token);

// ... diğer sözdizimi ile ilgili fonksiyonlar eklenebilir (örneğin, öncelik tabloları, AST yazdırma/debug fonksiyonları)


// --- Lexer/Parser Yardımcı Verileri (Bu modülde tutulabilir) ---
// Dışarıdan erişilmesi gerekiyorsa 'extern const' olarak bildirilir.
 extern const char *c_ampersand_keyword_strings[];
 extern const c_ampersand_token_type c_ampersand_keyword_types[];
// ... diğerleri

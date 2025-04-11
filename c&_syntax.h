#ifndef C_AMPERSAND_SYNTAX_H
#define C_AMPERSAND_SYNTAX_H

#include "c&_stdlib.h" // Temel tipler için
#include <stdbool.h>

// --- Token Tipleri ---
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,

    // Anahtar Kelimeler
    TOKEN_KEYWORD_FUNC,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_RETURN,

    // Operatörler
    TOKEN_OPERATOR_PLUS,
    TOKEN_OPERATOR_MINUS,
    TOKEN_OPERATOR_MULTIPLY,
    TOKEN_OPERATOR_DIVIDE,
    TOKEN_OPERATOR_ASSIGN,
    TOKEN_OPERATOR_EQUAL,
    TOKEN_OPERATOR_NOT_EQUAL,
    TOKEN_OPERATOR_LESS_THAN,
    TOKEN_OPERATOR_GREATER_THAN,

    // Ayraçlar ve Noktalama İşaretleri
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,

    // ... diğer token tipleri eklenebilir
    TOKEN_COUNT // Tüm token tiplerinin sayısını belirtmek için (kolaylık)
} c_ampersand_token_type;

// Bir token'ı temsil eden yapı
typedef struct c_ampersand_token {
    c_ampersand_token_type type;
    char *lexeme;       // Token'ın metin karşılığı
    int line;           // Token'ın bulunduğu satır
    int column;         // Token'ın bulunduğu sütun
    // ... diğer token bilgileri (değer, tür vb.) eklenebilir
} c_ampersand_token;

// --- Soyut Sözdizimi Ağacı (AST) Düğümleri ---

// Temel AST düğüm tipi
typedef enum {
    AST_NODE_PROGRAM,
    AST_NODE_FUNCTION_DECLARATION,
    AST_NODE_VARIABLE_DECLARATION,
    AST_NODE_ASSIGNMENT_EXPRESSION,
    AST_NODE_BINARY_EXPRESSION,
    AST_NODE_UNARY_EXPRESSION,
    AST_NODE_LITERAL,
    AST_NODE_IDENTIFIER,
    AST_NODE_BLOCK_STATEMENT,
    AST_NODE_IF_STATEMENT,
    AST_NODE_WHILE_STATEMENT,
    AST_NODE_RETURN_STATEMENT,
    AST_NODE_CALL_EXPRESSION,
    // ... diğer AST düğüm tipleri eklenebilir
    AST_NODE_COUNT
} c_ampersand_ast_node_type;

// Temel AST düğümü yapısı
typedef struct c_ampersand_ast_node {
    c_ampersand_ast_node_type type;
    // ... düğüme özgü veriler (örneğin, ifadeler için operatör, değişken bildirimi için isim vb.)
    struct c_ampersand_ast_node **children; // Alt düğümler
    size_t num_children;
    size_t children_capacity;
    c_ampersand_token *token; // Bu düğüme karşılık gelen token (isteğe bağlı)
} c_ampersand_ast_node;

// Yeni bir token oluşturur.
c_ampersand_token *c_ampersand_syntax_create_token(c_ampersand_token_type type, const char *lexeme, int line, int column);

// Yeni bir AST düğümü oluşturur.
c_ampersand_ast_node *c_ampersand_syntax_create_ast_node(c_ampersand_ast_node_type type, c_ampersand_token *token);

// Bir AST düğümüne alt düğüm ekler.
c_ampersand_result c_ampersand_syntax_add_child(c_ampersand_ast_node *parent, c_ampersand_ast_node *child);

// Bir AST düğümünü ve altındaki tüm düğümleri serbest bırakır.
void c_ampersand_syntax_free_ast_node(c_ampersand_ast_node *node);

// Bir token'ı serbest bırakır.
void c_ampersand_syntax_free_token(c_ampersand_token *token);

// ... diğer sözdizimi ile ilgili fonksiyonlar (örneğin, öncelik tabloları) eklenebilir

#endif // C_AMPERSAND_SYNTAX_H
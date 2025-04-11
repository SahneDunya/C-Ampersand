#ifndef C_AMPERSAND_MATCH_CODEGEN_H
#define C_AMPERSAND_MATCH_CODEGEN_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_syntax.h"       // AST düğüm tipleri için
#include "c&_types.h"       // Veri tipleri için

// Kod üretimi bağlamını tutan yapı (gerekirse)
typedef struct c_ampersand_codegen_context {
    // ... kod üretimi sırasında gereken bilgiler (örneğin, mevcut fonksiyon, label yönetimi)
} c_ampersand_codegen_context;

// Bir "match" ifadesi/deyimi için kod üretir.
c_ampersand_result c_ampersand_match_codegen(const c_ampersand_ast_node *match_node,
                                            struct c_ampersand_codegen_context *context);

#endif // C_AMPERSAND_MATCH_CODEGEN_H
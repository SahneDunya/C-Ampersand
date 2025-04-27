#ifndef C_AMPERSAND_MATCH_CODEGEN_H
#define C_AMPERSAND_MATCH_CODEGEN_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// AST düğüm tipleri için (c_ampersand_ast_node, AST_NODE_*)
#include "c&_syntax.h"
// C& veri tipleri için (c_ampersand_type_kind)
#include "c&_types.h"
// Sahne64 kod üretimi API tipleri ve fonksiyonları için
// Varsayım: Bu başlık, sahne64_type_t, sahne64_constant_t, sahne64_register_t,
// sahne64_label_t gibi tipleri ve sahne_codegen_emit_* fonksiyon prototiplerini içerir.
#include "sahne.h" // Sahne64 API'sı başlığı

// Kod üretimi bağlamını tutan yapı
// Bu struct, derleme sırasında gereken state bilgilerini içerir
// (örn: üretilen talimat listesi, sembol tablosu, register durumu, label haritası).
// Gerçek tanımı bu başlıkta veya ilgili başka bir yerde yapılmalıdır.
typedef struct c_ampersand_codegen_context {
    // Örnek alanlar:
     InstructionList *instructions; // Üretilen talimatların listesi
     SymbolTable *symbols;        // Değişkenler ve konumları
     RegisterAllocator *regs;     // Register tahsis durumu
     LabelMap *labels;            // Label isimleri ve konumları
    // ...
} c_ampersand_codegen_context;


// Bir "match" ifadesi/deyimi için kod üretir.
// match_node: Match AST düğümünün pointer'ı.
// context: Kod üretimi bağlamının pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_match_codegen(const c_ampersand_ast_node *match_node,
                                             struct c_ampersand_codegen_context *context);


#endif // C_AMPERSAND_MATCH_CODEGEN_H

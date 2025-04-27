#include "c&_match_codegen.h"
// AST düğüm tipleri için
#include "c&_syntax.h"
// C& veri tipleri için
#include "c&_types.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println için
#include "c&_stdlib.h"
// Sahne64 kod üretimi API tipleri ve fonksiyonları için
// Varsayım: Bu başlık, sahne64_type_t, sahne64_constant_t, sahne64_register_t,
// sahne64_label_t gibi tipleri ve sahne_codegen_emit_* fonksiyon prototiplerini içerir.
#include "sahne.h" // Sahne64 API'sı başlığı
#include <stdlib.h> // atoi için
#include <string.h> // strcmp için


// Varsayım: C_AMPERSAND_* hata kodları c&_stdlib.h veya c&_error_handling.h'de tanımlı
// Varsayım: c_ampersand_println fonksiyonu c&_stdlib.h'de bildirilmiş ve implemente edilmiştir.
// Varsayım: sahne64_type_t, sahne64_constant_t, sahne64_register_t, sahne64_label_t sahne.h'de tanımlı.
// Varsayım: sahne_codegen_emit_* gibi kod üretimi fonksiyonları sahne.h'de bildirilmiş ve context alır.
// Varsayım: SAHNE64_INVALID_REGISTER gibi sabitler sahne.h'de tanımlı.
// Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de tanımlı.


// --- Yardımcı Fonksiyonlar ---

// Bir C& AST düğümünün türünü hipotetik Sahne64 tip sistemine çevirir.
static sahne64_type_t map_type_to_sahne64(c_ampersand_type_kind kind) {
    switch (kind) {
        case TYPE_VOID:    return SAHNE64_TYPE_VOID;
        case TYPE_BOOL:    return SAHNE64_TYPE_BOOL;
        case TYPE_INT8:    return SAHNE64_TYPE_INT8;
        case TYPE_INT16:   return SAHNE64_TYPE_INT16;
        case TYPE_INT32:   return SAHNE64_TYPE_INT32;
        case TYPE_INT64:   return SAHNE64_TYPE_INT64;
        case TYPE_UINT8:   return SAHNE64_TYPE_UINT8;
        case TYPE_UINT16:  return SAHNE64_TYPE_UINT16;
        case TYPE_UINT32:  return SAHNE64_TYPE_UINT32;
        case TYPE_UINT64:  return SAHNE64_TYPE_UINT64;
        case TYPE_FLOAT32: return SAHNE64_TYPE_FLOAT32;
        case TYPE_FLOAT64: return SAHNE64_TYPE_FLOAT64;
        case TYPE_STRING:  return SAHNE64_TYPE_POINTER; // Stringler pointer olarak temsil edilebilir
        case TYPE_POINTER: return SAHNE64_TYPE_POINTER;
        // ... diğer C& türleri için eşlemeler (Sahne64 Handle, TaskId vb.)
        default:
            c_ampersand_println("CODEGEN HATA: Bilinmeyen C& tipi %d", kind);
            return SAHNE64_TYPE_UNKNOWN; // Bilinmeyen tip
    }
}

// Bir AST literal düğümünden değeri alır ve hipotetik Sahne64 için bir sabite dönüştürür.
static sahne64_constant_t get_literal_value(const c_ampersand_ast_node *literal_node) {
    // Varsayım: sahne64_constant_t bir union içerir ve farklı tipler için alanları vardır.
    sahne64_constant_t constant = {0}; // Tüm alanları sıfırla (union dahil)

    if (literal_node != NULL && literal_node->token != NULL) {
        // Token tipine göre literal değeri al ve Sahne64 sabiti olarak ayarla
        if (literal_node->token->type == TOKEN_INTEGER_LITERAL) {
            constant.type = SAHNE64_TYPE_INT32; // Varsayım: Tüm int literaller şimdilik int32
            constant.value.i32 = atoi(literal_node->token->lexeme); // atoi kullanımı, no_std ortamında mevcutsa
        } else if (literal_node->token->type == TOKEN_BOOL_LITERAL) {
            constant.type = SAHNE64_TYPE_BOOL;
            constant.value.boolean = (strcmp(literal_node->token->lexeme, "true") == 0);
        }
        // ... diğer literal tipleri için eklemeler (float, string, char vb.)
        // String literaller daha karmaşık olabilir (veri bölümünde saklanıp pointer kullanma)
    } else {
        // Geçersiz literal düğümü
        constant.type = SAHNE64_TYPE_UNKNOWN; // Hata durumu
    }
    return constant;
}

// Bir ifade için kod üretir ve sonucunu bir register'da bırakır (hipotetik).
// context: Kod üretim bağlamı.
// expression_node: İfade AST düğümü.
// Dönüş: Sonucun tutulduğu hipotetik Sahne64 register'ı veya hata/geçersiz register.
static sahne64_register_t codegen_expression(const c_ampersand_ast_node *expression_node,
                                               struct c_ampersand_codegen_context *context) {
    // Varsayım: context NULL değil ve gerekli alanları (örn. register allocator) içeriyor.
    if (context == NULL) {
        c_ampersand_println("CODEGEN HATA: Geçersiz kod üretim bağlamı (NULL).");
        return SAHNE64_INVALID_REGISTER;
    }
    sahne64_register_t result_reg = SAHNE64_INVALID_REGISTER;

    if (expression_node == NULL) {
        c_ampersand_println("CODEGEN HATA: NULL ifade düğümü.");
        return result_reg; // Geçersiz düğüm
    }

    switch (expression_node->type) {
        case AST_NODE_IDENTIFIER:
            // Değişkenin değerini bir register'a yükle
            // result_reg = sahne_codegen_emit_load_variable(context, expression_node->token->lexeme);
            c_ampersand_println("CODEGEN NOT: Değişken '%s' için yükleme kodu üretilecek.", expression_node->token->lexeme);
            // Gerçek kod üretiminde, değişkenin sembol tablosundan konumu (register veya bellek) bulunur
             // ve ilgili yükleme talimatı (örn. MOV) üretilir. Register tahsisi yapılır.
             result_reg = SAHNE64_INVALID_REGISTER; // Placeholder
            break;
        case AST_NODE_LITERAL: {
            sahne64_constant_t constant = get_literal_value(expression_node);
            if (constant.type == SAHNE64_TYPE_UNKNOWN) {
                 c_ampersand_println("CODEGEN HATA: Desteklenmeyen literal türü.");
                 return SAHNE64_INVALID_REGISTER;
            }
            // Sabiti bir register'a yükle
            // result_reg = sahne_codegen_emit_load_constant(context, constant);
            c_ampersand_println("CODEGEN NOT: Sabit '%s' için yükleme kodu üretilecek.", expression_node->token->lexeme);
            // Gerçek kod üretiminde, sabit değeri doğrudan register'a yükleme talimatı (örn. MOVI) üretilir.
            result_reg = SAHNE64_INVALID_REGISTER; // Placeholder
            break;
        }
        // ... diğer ifade türleri için kod üretimi (örn. ikili operasyonlar, fonksiyon çağrıları)
        // case AST_NODE_BINARY_OP:
        //    sahne64_register_t left_reg = codegen_expression(expression_node->children[0], context);
        //    sahne64_register_t right_reg = codegen_expression(expression_node->children[1], context);
        //    result_reg = sahne_codegen_emit_binary_operation(context, expression_node->token->type, left_reg, right_reg);
        //    sahne_codegen_release_register(context, left_reg); // Geçici registerları serbest bırak
        //    sahne_codegen_release_register(context, right_reg);
        //    break;
        // ...

        default:
            c_ampersand_println("CODEGEN HATA: Desteklenmeyen ifade türü %d için kod üretimi.", expression_node->type);
            return SAHNE64_INVALID_REGISTER; // Hata
    }

    // Register tahsis başarılı olduysa döndür
    return result_reg;
}

// Bir deyim için kod üretir (hipotetik).
// context: Kod üretim bağlamı.
// statement_node: Deyim AST düğümü.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
static c_ampersand_result codegen_statement(const c_ampersand_ast_node *statement_node,
                                            struct c_ampersand_codegen_context *context) {
    if (context == NULL) {
         c_ampersand_println("CODEGEN HATA: Geçersiz kod üretim bağlamı (NULL).");
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    if (statement_node == NULL) {
        c_ampersand_println("CODEGEN HATA: NULL deyim düğümü.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT}; // Geçersiz düğüm
    }

    switch (statement_node->type) {
        case AST_NODE_BLOCK_STATEMENT: // Süslü parantezler içindeki blok
            c_ampersand_println("CODEGEN NOT: Blok deyimi için kod üretiliyor.");
            for (size_t i = 0; i < statement_node->num_children; ++i) {
                c_ampersand_result result = codegen_statement(statement_node->children[i], context); // Çocuk deyimleri özyinelemeli derle
                if (result.code != C_AMPERSAND_OK) {
                    return result; // Alt deyimde hata olursa dur ve hatayı ilet
                }
            }
            c_ampersand_println("CODEGEN NOT: Blok deyimi tamamlandı.");
            break;
        // ... diğer deyim türleri için kod üretimi (örn. atama, if, while, return, fonksiyon çağrısı deyimi)
        // case AST_NODE_ASSIGNMENT:
        //    c_ampersand_println("CODEGEN NOT: Atama deyimi için kod üretiliyor.");
        //    // Sağ taraf ifadesi için kod üret
        //    sahne64_register_t value_reg = codegen_expression(statement_node->children[1], context);
        //    if (value_reg == SAHNE64_INVALID_REGISTER) return (c_ampersand_result){C_AMPERSAND_ERROR_CODEGEN_FAILED};
        //    // Değeri sol taraftaki değişkene/konuma sakla
        //    // sahne_codegen_emit_store_variable(context, statement_node->children[0]->token->lexeme, value_reg);
        //    c_ampersand_println("CODEGEN NOT: Değeri '%s' değişkenine saklama kodu üretilecek.", statement_node->children[0]->token->lexeme);
        //    sahne_codegen_release_register(context, value_reg); // Geçici registerı serbest bırak
        //    break;
        // ...

        default:
            c_ampersand_println("CODEGEN NOT: Desteklenmeyen deyim türü %d için kod üretimi atlanıyor.", statement_node->type);
            // Desteklenmeyen bir deyim türü için kod üretmek hata olabilir veya atlanabilir.
            // Şimdilik atlayıp uyarı verelim. Gerçek bir derleyicide bu bir hata olmalıdır.
            // return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION}; // Hata dönmek isterseniz
            break; // Devam et
    }
    return C_AMPERSAND_OK_RESULT; // Başarı
}


// --- "match" İfadesi/Deyimi için Kod Üretimi ---

c_ampersand_result c_ampersand_match_codegen(const c_ampersand_ast_node *match_node,
                                            struct c_ampersand_codegen_context *context) {

    // context ve AST düğümü geçerlilik kontrolü
    if (context == NULL || match_node == NULL || match_node->type != AST_NODE_MATCH_EXPRESSION) {
        c_ampersand_println("CODEGEN HATA: Geçersiz match düğümü veya bağlam.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    // Match ifadesinin en az bir ifade ve bir kolu olmalı
    if (match_node->num_children < 2) {
        c_ampersand_println("CODEGEN HATA: Match ifadesinde eksik ifade veya kol.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX};
    }

    const c_ampersand_ast_node *expression_to_match = match_node->children[0];
    // Diğer çocuklar match kollarıdır (pattern ve ifade/blok)

    // Eşlenecek ifade için kod üret ve sonucunu bir register'da sakla
    c_ampersand_println("CODEGEN NOT: Match ifadesi için kod üretiliyor...");
    sahne64_register_t match_value_reg = codegen_expression(expression_to_match, context);
    if (match_value_reg == SAHNE64_INVALID_REGISTER) {
        c_ampersand_println("CODEGEN HATA: Match ifadesi kodu üretilemedi.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_CODEGEN_FAILED};
    }

    // "match" yapısının sonunu işaretlemek için bir label oluştur
    // Bu label, eşleşen bir kol çalıştıktan sonra atlanacak noktayı belirler.
    // sahne64_label_t match_end_label = sahne_codegen_create_label(context, "match_end");
    c_ampersand_println("CODEGEN NOT: Match sonu için label oluşturulacak.");


    // Her bir match kolu için kod üret
    for (size_t i = 1; i < match_node->num_children; ++i) {
        const c_ampersand_ast_node *match_arm = match_node->children[i];
        // Match kolu düğümünü ve çocuk sayısını doğrula
        if (match_arm->type != AST_NODE_MATCH_ARM || match_arm->num_children != 2) {
            c_ampersand_println("CODEGEN HATA: Geçersiz match kolu yapısı.");
            // Şu ana kadar üretilen kodu (match_value_reg için load gibi) geri almak gerekebilir.
            // Basitlik adına bu yapılmıyor.
            // Sahne64_release_register(context, match_value_reg); // Registerı serbest bırak (eğer codegen_expression başarılıysa)
            return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX};
        }

        const c_ampersand_ast_node *pattern_node = match_arm->children[0]; // Kolun pattern'i
        const c_ampersand_ast_node *body_node = match_arm->children[1];   // Kolun çalışacak body'si

        // Pattern için kod üret (şu an sadece literal değerlerle karşılaştırma destekleniyor)
        if (pattern_node->type == AST_NODE_LITERAL) {
            // Literal pattern değerini hipotetik Sahne64 sabitine çevir
            sahne64_constant_t pattern_value = get_literal_value(pattern_node);
             if (pattern_value.type == SAHNE64_TYPE_UNKNOWN) {
                 c_ampersand_println("CODEGEN HATA: Desteklenmeyen literal pattern türü.");
                 // Sahne64_release_register(context, match_value_reg); // Registerı serbest bırak
                 return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
            }

            // Match ifadesinin sonucunu (match_value_reg) pattern değeriyle karşılaştır
            // sahne_codegen_emit_compare_register_with_constant(context, match_value_reg, pattern_value);
            c_ampersand_println("CODEGEN NOT: Match değeri (Reg %d) ile pattern (%s) karşılaştırma kodu üretilecek.",
                                 match_value_reg, pattern_node->token->lexeme);


            // Eşleşme durumunda kolun body'sine atlamak için bir label oluştur
            // sahne64_label_t arm_label = sahne_codegen_create_label(context, "match_arm_%zu", i);
            c_ampersand_println("CODEGEN NOT: Match kolu %zu için label oluşturulacak.", i);
            // sahne_codegen_emit_jump_if_equal(context, arm_label); // Karşılaştırma sonucu eşitse atla
            c_ampersand_println("CODEGEN NOT: Eşleşirse kola atlama kodu üretilecek.");

            // Eğer eşleşme olmazsa (koşullu atlama gerçekleşmezse), bir sonraki kola atlamak için
            // koşulsuz bir atlama talimatı üretilir. Bu, switch/if-else if yapısına denk gelir.
            // Son kol için bu atlamaya gerek yoktur.
            if (i < match_node->num_children - 1) {
                 // sahne64_label_t next_arm_label = sahne_codegen_create_label(context, "match_arm_cond_%zu", i+1); // Bir sonraki kolun koşul label'ı
                 // sahne_codegen_emit_jump(context, next_arm_label); // Eşleşmediyse bir sonraki kolun başına atla
                 c_ampersand_println("CODEGEN NOT: Eşleşmezse bir sonraki kola atlama kodu üretilecek.");
                 // sahne_codegen_define_label(context, next_arm_label); // Bir sonraki kolun koşul label'ını tanımla
            }


            // Kolun label'ını tanımla (eşleşme durumunda atlanacak nokta)
            // sahne_codegen_define_label(context, arm_label);
            c_ampersand_println("CODEGEN NOT: Kol %zu body label'ı tanımlanacak.", i);

            // Kolun body'si (deyimi veya bloğu) için kod üret
            c_ampersand_result body_result = codegen_statement(body_node, context);
            if (body_result.code != C_AMPERSAND_OK) {
                 // Body kod üretiminde hata olursa temizlik yapıp hatayı ilet
                 // Sahne64_release_register(context, match_value_reg); // Registerı serbest bırak
                return body_result;
            }

            // Kolun body'si bittikten sonra (eğer fallthrough yoksa) match sonuna atla
            // sahne_codegen_emit_jump(context, match_end_label);
            c_ampersand_println("CODEGEN NOT: Kol %zu sonundan match sonuna atlama kodu üretilecek.", i);

        } else {
            c_ampersand_println("CODEGEN HATA: Desteklenmeyen pattern türü %d.", pattern_node->type);
            // Desteklenmeyen pattern türü durumunda hata
            // Sahne64_release_register(context, match_value_reg); // Registerı serbest bırak
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
        }
    } // Match kolları döngüsü sonu


    // Match sonu label'ını tanımla (tüm kolların atladığı veya son kolun bittiği yer)
    // sahne_codegen_define_label(context, match_end_label);
    c_ampersand_println("CODEGEN NOT: Match sonu label'ı tanımlanacak.");

    // Match ifadesinin sonucunu (eğer match bir ifade olarak kullanılıyorsa) bir register'da bırak.
    // Şu anki match yapısı deyim gibi ele alınmış, ama ifade olarak da sonuç döndürebilir.
    // Eğer ifade olsaydı, her kolun body'sinin sonucunu aynı register'a taşıması gerekirdi.
    // match_value_reg register'ını serbest bırak
    // sahne_codegen_release_register(context, match_value_reg);
    c_ampersand_println("CODEGEN NOT: Match ifadesi sonucu register'ı serbest bırakılacak.");


    c_ampersand_println("CODEGEN NOT: Match kodu üretimi tamamlandı.");
    return C_AMPERSAND_OK_RESULT; // Başarı
}

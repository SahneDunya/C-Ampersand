#include "c&_match_codegen.h"
#include "c&_syntax.h"
#include "c&_types.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Kod üretimi için Sahne64 API'sı
#include <stdio.h>
#include <stdlib.h>

// --- Yardımcı Fonksiyonlar ---

// Bir AST düğümünün türünü Sahne64'ün tip sistemine çevirir (gerekirse).
static sahne64_type_t map_type_to_sahne64(c_ampersand_type_kind kind) {
    switch (kind) {
        case TYPE_INT32:
            return SAHNE64_TYPE_INT32;
        case TYPE_BOOL:
            return SAHNE64_TYPE_BOOL;
        // ... diğer türler için eşlemeler
        default:
            return SAHNE64_TYPE_UNKNOWN;
    }
}

// Bir AST düğümünden literal değeri alır ve Sahne64 için bir sabite dönüştürür (gerekirse).
static sahne64_constant_t get_literal_value(const c_ampersand_ast_node *literal_node) {
    sahne64_constant_t constant = {0};
    if (literal_node != NULL && literal_node->token != NULL) {
        // Token tipine göre literal değeri al
        if (literal_node->token->type == TOKEN_INTEGER_LITERAL) {
            constant.type = SAHNE64_TYPE_INT32;
            constant.value.i32 = atoi(literal_node->token->lexeme);
        } else if (literal_node->token->type == TOKEN_BOOL_LITERAL) {
            constant.type = SAHNE64_TYPE_BOOL;
            constant.value.boolean = (strcmp(literal_node->token->lexeme, "true") == 0);
        }
        // ... diğer literal tipleri için eklemeler
    }
    return constant;
}

// Bir ifade için kod üretir ve sonucunu bir register'da bırakır.
static sahne64_register_t codegen_expression(const c_ampersand_ast_node *expression_node,
                                               struct c_ampersand_codegen_context *context) {
    sahne64_register_t result_reg = SAHNE64_INVALID_REGISTER;
    if (expression_node == NULL) {
        return result_reg;
    }

    switch (expression_node->type) {
        case AST_NODE_IDENTIFIER:
            // Değişkenin değerini yükle
            // result_reg = sahne64_load_variable(expression_node->token->lexeme, context);
            fprintf(stderr, "// TODO: Değişken yükleme kodu üretilecek\n");
            break;
        case AST_NODE_LITERAL: {
            sahne64_constant_t constant = get_literal_value(expression_node);
            // Sabiti bir register'a yükle
            // result_reg = sahne64_load_constant(constant, context);
            fprintf(stderr, "// TODO: Sabit yükleme kodu üretilecek: %s\n", expression_node->token->lexeme);
            break;
        }
        // ... diğer ifade türleri için kod üretimi
        default:
            fprintf(stderr, "// Hata: İfade için kod üretimi desteklenmiyor\n");
            break;
    }
    return result_reg;
}

// Bir deyim için kod üretir.
static c_ampersand_result codegen_statement(const c_ampersand_ast_node *statement_node,
                                            struct c_ampersand_codegen_context *context) {
    if (statement_node == NULL) {
        return C_AMPERSAND_OK_RESULT;
    }

    switch (statement_node->type) {
        case AST_NODE_BLOCK_STATEMENT:
            for (size_t i = 0; i < statement_node->num_children; ++i) {
                c_ampersand_result result = codegen_statement(statement_node->children[i], context);
                if (result.code != C_AMPERSAND_OK) {
                    return result;
                }
            }
            break;
        // ... diğer deyim türleri için kod üretimi
        default:
            fprintf(stderr, "// TODO: Deyim için kod üretimi: %d\n", statement_node->type);
            break;
    }
    return C_AMPERSAND_OK_RESULT;
}

// --- "match" İfadesi/Deyimi için Kod Üretimi ---

c_ampersand_result c_ampersand_match_codegen(const c_ampersand_ast_node *match_node,
                                            struct c_ampersand_codegen_context *context) {
    if (match_node == NULL || match_node->type != AST_NODE_MATCH_EXPRESSION) { // AST_NODE_MATCH_EXPRESSION varsayımı
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    if (match_node->num_children < 2) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX};
    }

    const c_ampersand_ast_node *expression_to_match = match_node->children[0];
    // Diğer çocuklar match kollarıdır (pattern ve ifade/blok)

    // Eşlenecek ifade için kod üret ve sonucu bir register'da sakla
    sahne64_register_t match_value_reg = codegen_expression(expression_to_match, context);
    if (match_value_reg == SAHNE64_INVALID_REGISTER) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_CODEGEN_FAILED};
    }

    // "match" yapısının sonunu işaretlemek için bir label oluştur
    // sahne64_label_t match_end_label = sahne64_create_label(context, "match_end");
    fprintf(stderr, "// TODO: Match sonu için label oluşturulacak\n");

    // Her bir match kolu için kod üret
    for (size_t i = 1; i < match_node->num_children; ++i) {
        const c_ampersand_ast_node *match_arm = match_node->children[i];
        if (match_arm->type != AST_NODE_MATCH_ARM || match_arm->num_children != 2) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX};
        }

        const c_ampersand_ast_node *pattern_node = match_arm->children[0];
        const c_ampersand_ast_node *body_node = match_arm->children[1];

        // Pattern için kod üret (sabit bir değerle karşılaştırma varsayımı)
        if (pattern_node->type == AST_NODE_LITERAL) {
            sahne64_constant_t pattern_value = get_literal_value(pattern_node);

            // Değeri match ifadesinin sonucuyla karşılaştır
            // sahne64_compare_register_with_constant(match_value_reg, pattern_value, context);
            fprintf(stderr, "// TODO: Değer karşılaştırma kodu üretilecek\n");

            // Eşleşme durumunda body'ye atlamak için bir label oluştur
            // sahne64_label_t arm_label = sahne64_create_label(context, "match_arm_%zu", i);
            fprintf(stderr, "// TODO: Match kolu için label oluşturulacak\n");
            // sahne64_jump_if_equal(arm_label, context);
            fprintf(stderr, "// TODO: Eşleşirse kola atlama kodu üretilecek\n");

            // Bir sonraki kola geçmek için atlama (eğer fallthrough yoksa)
            // sahne64_jump(match_end_label, context);
            fprintf(stderr, "// TODO: Bir sonraki kola atlama kodu üretilecek\n");

            // Kolun label'ını tanımla
            // sahne64_define_label(arm_label, context);
            fprintf(stderr, "// TODO: Kol label'ı tanımlanacak\n");

            // Kolun body'si için kod üret
            c_ampersand_result body_result = codegen_statement(body_node, context);
            if (body_result.code != C_AMPERSAND_OK) {
                return body_result;
            }

            // Kolun sonundan sonra (eğer fallthrough yoksa) match sonuna atla
            // sahne64_jump(match_end_label, context);
            fprintf(stderr, "// TODO: Match sonuna atlama kodu üretilecek (kol sonu)\n");
        } else {
            fprintf(stderr, "// Hata: Desteklenmeyen pattern türü\n");
            return (c_ampersand_result){C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION};
        }
    }

    // Match sonu label'ını tanımla
    // sahne64_define_label(match_end_label, context);
    fprintf(stderr, "// TODO: Match sonu label'ı tanımlanacak\n");

    // Match ifadesinin sonucunu (eğer varsa) bir register'da bırak (örnekte ihmal edilmiştir)
    // sahne64_release_register(match_value_reg, context);
    fprintf(stderr, "// TODO: Match sonucu register'ı serbest bırakılacak\n");

    return C_AMPERSAND_OK_RESULT;
}
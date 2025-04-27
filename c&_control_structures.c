#include "c&_control_structures.h"
#include "c&_types.h"      // Varsayım: Genel tip tanımları
#include "c&_data_types.h" // Varsayım: c_ampersand_value tanımı
#include "c&_stdlib.h"     // Varsayım: c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT tanımları

#include <stdbool.h>   // bool, true, false için
#include <stddef.h>    // size_t, NULL için


// --- Kontrol Yapısı Implementasyonları ---

c_ampersand_result c_ampersand_control_execute_if(const c_ampersand_ast_node *if_node,
                                                  c_ampersand_evaluate_expression_func evaluate_expression,
                                                  c_ampersand_execute_statement_func execute_statement) {
    // AST düğüm tipi, çocuk sayısı ve fonksiyon pointer'ı null kontrolü
    if (if_node == NULL || if_node->type != AST_NODE_IF_STATEMENT ||
        if_node->num_children < 2 || if_node->num_children > 3 ||
        evaluate_expression == NULL || execute_statement == NULL) {
        // Varsayım: C_AMPERSAND_ERROR_INVALID_ARGUMENT c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    const c_ampersand_ast_node *condition_node = if_node->children[0];
    const c_ampersand_ast_node *then_block_node = if_node->children[1];
    const c_ampersand_ast_node *else_block_node = (if_node->num_children == 3) ? if_node->children[2] : NULL;

    c_ampersand_value condition_result;
    c_ampersand_result eval_result = evaluate_expression(condition_node, &condition_result);
    // Varsayım: c_ampersand_result struct içinde 'code' alanı ve C_AMPERSAND_OK c&_stdlib.h'de tanımlı
    if (eval_result.code != C_AMPERSAND_OK) {
        return eval_result; // İfade değerlendirme hatasını doğrudan ilet
    }

    // Varsayım: c_ampersand_value struct içinde 'type' structı, 'kind' alanı ve TYPE_BOOL c&_data_types.h'de tanımlı
    if (condition_result.type.kind != TYPE_BOOL) {
        // Varsayım: C_AMPERSAND_ERROR_TYPE_MISMATCH c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }

    // Varsayım: c_ampersand_value struct içinde 'data' union/structı ve 'bool_val' alanı tanımlı
    if (condition_result.data.bool_val) {
        return execute_statement(then_block_node);
    } else if (else_block_node != NULL) {
        return execute_statement(else_block_node);
    }

    // Varsayım: C_AMPERSAND_OK_RESULT c&_stdlib.h'de tanımlı (Genellikle { C_AMPERSAND_OK } veya { SAHNE_SUCCESS } olur)
    return C_AMPERSAND_OK_RESULT; // Hiçbir blok yürütülmediyse başarı dön
}

c_ampersand_result c_ampersand_control_execute_while(const c_ampersand_ast_node *while_node,
                                                   c_ampersand_evaluate_expression_func evaluate_expression,
                                                   c_ampersand_execute_statement_func execute_statement) {
     // AST düğüm tipi, çocuk sayısı ve fonksiyon pointer'ı null kontrolü
    if (while_node == NULL || while_node->type != AST_NODE_WHILE_STATEMENT ||
        while_node->num_children != 2 ||
        evaluate_expression == NULL || execute_statement == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    const c_ampersand_ast_node *condition_node = while_node->children[0];
    const c_ampersand_ast_node *body_node = while_node->children[1];

    while (true) { // Sonsuz döngü, koşul içeride kontrol ediliyor
        c_ampersand_value condition_result;
        c_ampersand_result eval_result = evaluate_expression(condition_node, &condition_result);
        if (eval_result.code != C_AMPERSAND_OK) {
            return eval_result; // Koşul değerlendirme hatasını doğrudan ilet
        }

        if (condition_result.type.kind != TYPE_BOOL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
        }

        if (!condition_result.data.bool_val) {
            break; // Koşul yanlışsa döngüden çık
        }

        c_ampersand_result execute_result = execute_statement(body_node);
        if (execute_result.code != C_AMPERSAND_OK) {
            // İfade yürütme hatasını veya return gibi özel bir durumu yakala ve ilet
            // Not: return, break, continue gibi kontrol akışı ifadeleri burada özel olarak ele alınmalıdır.
            // Mevcut basit implementasyon sadece hatayı iletiyor.
            return execute_result;
        }
    }

    return C_AMPERSAND_OK_RESULT; // Döngü başarıyla tamamlandıysa başarı dön
}

c_ampersand_result c_ampersand_control_execute_return(const c_ampersand_ast_node *return_node,
                                                      c_ampersand_evaluate_expression_func evaluate_expression,
                                                      c_ampersand_value *return_value) {
    // AST düğüm tipi ve fonksiyon pointer'ı null kontrolü
    if (return_node == NULL || return_node->type != AST_NODE_RETURN_STATEMENT || evaluate_expression == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

     // return_value pointer'ının NULL olmaması gerektiğini varsayıyoruz eğer dönüş değeri bekleniyorsa.
     // Void dönüş durumunda NULL olabilir.
     if (return_node->num_children == 1 && return_value == NULL) {
         return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
     }


    if (return_node->num_children == 1) {
        // Dönüş değeri olan return
        return evaluate_expression(return_node->children[0], return_value);
        // Not: Çağıranın (execute_statement veya fonksiyon yürütücünün) bu dönüş değerini ve
        // 'return' durumunu c_ampersand_result kodundan veya özel bir mekanizmadan
        // anlayıp uygun şekilde işlemesi gerekir.
    } else if (return_node->num_children == 0) {
        // Void dönüş tipi (return;)
        // return_value NULL ise sorun yok, değilse içeriği tanımsız olabilir.
        return C_AMPERSAND_OK_RESULT;
         // Not: Çağıranın bu durumu yakalayıp fonksiyon yürütmeyi sonlandırması gerekir.
         // Özel bir dönüş kodu (örn. C_AMPERSAND_SPECIAL_RETURN) burada daha uygun olabilir.
    } else {
        // Varsayım: C_AMPERSAND_ERROR_INVALID_SYNTAX c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX}; // return'ün 0 veya 1 çocuğu olmalı
    }
}

c_ampersand_result c_ampersand_control_execute_block(const c_ampersand_ast_node *block_node,
                                                   c_ampersand_execute_statement_func execute_statement) {
     // AST düğüm tipi ve fonksiyon pointer'ı null kontrolü
    if (block_node == NULL || block_node->type != AST_NODE_BLOCK_STATEMENT || execute_statement == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    // Blok içindeki her ifadeyi sırayla yürüt
    for (size_t i = 0; i < block_node->num_children; ++i) {
        c_ampersand_result result = execute_statement(block_node->children[i]);
        if (result.code != C_AMPERSAND_OK) {
            // Eğer yürütülen ifade bir hata döndürürse (veya return/break/continue gibi
            // özel bir durumsa, buradaki logic daha karmaşık olmalıydı),
            // yürütmeyi durdur ve sonucu döndür.
            return result;
        }
    }

    return C_AMPERSAND_OK_RESULT; // Tüm blok başarıyla yürütüldü
}

// ... diğer kontrol yapıları için implementasyonlar buraya eklenebilir (örneğin for döngüsü)

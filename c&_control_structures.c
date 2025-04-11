#include "c&_control_structures.h"
#include "c&_types.h"
#include "c&_data_types.h"
#include "c&_stdlib.h"
#include "sahne64_api.h" // Gerekirse sistem çağrıları için (örneğin, uyku)
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Kontrol Yapısı Implementasyonları ---

c_ampersand_result c_ampersand_control_execute_if(const c_ampersand_ast_node *if_node,
                                                 c_ampersand_evaluate_expression_func evaluate_expression,
                                                 c_ampersand_execute_statement_func execute_statement) {
    if (if_node == NULL || if_node->type != AST_NODE_IF_STATEMENT ||
        if_node->num_children < 2 || if_node->num_children > 3) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    const c_ampersand_ast_node *condition_node = if_node->children[0];
    const c_ampersand_ast_node *then_block_node = if_node->children[1];
    const c_ampersand_ast_node *else_block_node = (if_node->num_children == 3) ? if_node->children[2] : NULL;

    c_ampersand_value condition_result;
    c_ampersand_result eval_result = evaluate_expression(condition_node, &condition_result);
    if (eval_result.code != C_AMPERSAND_OK) {
        return eval_result;
    }

    if (condition_result.type.kind != TYPE_BOOL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
    }

    if (condition_result.data.bool_val) {
        return execute_statement(then_block_node);
    } else if (else_block_node != NULL) {
        return execute_statement(else_block_node);
    }

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_control_execute_while(const c_ampersand_ast_node *while_node,
                                                    c_ampersand_evaluate_expression_func evaluate_expression,
                                                    c_ampersand_execute_statement_func execute_statement) {
    if (while_node == NULL || while_node->type != AST_NODE_WHILE_STATEMENT ||
        while_node->num_children != 2) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    const c_ampersand_ast_node *condition_node = while_node->children[0];
    const c_ampersand_ast_node *body_node = while_node->children[1];

    while (true) {
        c_ampersand_value condition_result;
        c_ampersand_result eval_result = evaluate_expression(condition_node, &condition_result);
        if (eval_result.code != C_AMPERSAND_OK) {
            return eval_result;
        }

        if (condition_result.type.kind != TYPE_BOOL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_TYPE_MISMATCH};
        }

        if (!condition_result.data.bool_val) {
            break;
        }

        c_ampersand_result execute_result = execute_statement(body_node);
        if (execute_result.code != C_AMPERSAND_OK) {
            return execute_result;
        }
    }

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_control_execute_return(const c_ampersand_ast_node *return_node,
                                                     c_ampersand_evaluate_expression_func evaluate_expression,
                                                     c_ampersand_value *return_value) {
    if (return_node == NULL || return_node->type != AST_NODE_RETURN_STATEMENT) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    if (return_node->num_children == 1) {
        return evaluate_expression(return_node->children[0], return_value);
    } else if (return_node->num_children == 0) {
        // Void dönüş tipi
        return C_AMPERSAND_OK_RESULT;
    } else {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_SYNTAX};
    }
}

c_ampersand_result c_ampersand_control_execute_block(const c_ampersand_ast_node *block_node,
                                                    c_ampersand_execute_statement_func execute_statement) {
    if (block_node == NULL || block_node->type != AST_NODE_BLOCK_STATEMENT) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    for (size_t i = 0; i < block_node->num_children; ++i) {
        c_ampersand_result result = execute_statement(block_node->children[i]);
        if (result.code != C_AMPERSAND_OK) {
            return result;
        }
        // Burada return ifadesiyle karşılaşıldığında döngüden çıkılabilir (fonksiyonlardan dönüş).
        // Bu örnekte basitçe tüm ifadeler yürütülür.
    }

    return C_AMPERSAND_OK_RESULT;
}

// ... diğer kontrol yapıları için implementasyonlar buraya eklenebilir
#include "c&_semantics.h"
#include "c&_stdlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// --- Sembol Tablosu Yönetimi ---

c_ampersand_symbol_table *c_ampersand_semantics_create_symbol_table(c_ampersand_symbol_table *parent) {
    c_ampersand_symbol_table *table = malloc(sizeof(c_ampersand_symbol_table));
    if (table == NULL) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

c_ampersand_result c_ampersand_semantics_add_symbol(c_ampersand_semantic_context *context, const char *name, c_ampersand_type type) {
    if (context == NULL || context->current_scope == NULL || name == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_symbol_entry *new_entry = malloc(sizeof(c_ampersand_symbol_entry));
    if (new_entry == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
    }
    new_entry->name = strdup(name);
    new_entry->type = type;
    new_entry->next = context->current_scope->head;
    context->current_scope->head = new_entry;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_symbol_entry *c_ampersand_semantics_lookup_symbol(c_ampersand_semantic_context *context, const char *name) {
    if (context == NULL || name == NULL) {
        return NULL;
    }

    c_ampersand_symbol_table *current_table = context->current_scope;
    while (current_table != NULL) {
        c_ampersand_symbol_entry *current_entry = current_table->head;
        while (current_entry != NULL) {
            if (strcmp(current_entry->name, name) == 0) {
                return current_entry;
            }
            current_entry = current_entry->next;
        }
        // Üst kapsamı kontrol etmek için (basit örnekte üst kapsam yok)
        current_table = NULL; // Gerçekte üst kapsam bağlantısı olmalı
    }
    return NULL;
}

// --- Tip Kontrolü ---

bool c_ampersand_semantics_check_type_compatibility(c_ampersand_type type1, c_ampersand_type type2) {
    return type1 == type2; // Basit bir eşitlik kontrolü
}

// --- Hata Raporlama ---

void c_ampersand_semantics_report_error(c_ampersand_ast_node *node, const char *message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, "Semantik Hata");
    if (node != NULL && node->token != NULL) {
        fprintf(stderr, " (Satır: %d, Sütun: %d)", node->token->line, node->token->column);
    }
    fprintf(stderr, ": ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// --- AST Üzerinde Semantik Analiz ---

static c_ampersand_result analyze_node(c_ampersand_ast_node *node, c_ampersand_semantic_context *context);

static c_ampersand_result analyze_program(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    for (size_t i = 0; i < node->num_children; ++i) {
        c_ampersand_result result = analyze_node(node->children[i], context);
        if (result.code != C_AMPERSAND_OK) {
            return result;
        }
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result analyze_variable_declaration(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    if (node->num_children != 2 || node->children[0]->type != AST_NODE_IDENTIFIER) {
        c_ampersand_semantics_report_error(node, "Geçersiz değişken bildirimi.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC};
    }
    c_ampersand_token *identifier_token = node->children[0]->token;
    // Burada tip bilgisi de olmalı, örnekte atlanmıştır.
    c_ampersand_semantics_add_symbol(context, identifier_token->lexeme, TYPE_INT); // Örnek tip
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result analyze_binary_expression(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    if (node->num_children != 2) {
        c_ampersand_semantics_report_error(node, "Geçersiz ikili ifade.");
        return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC};
    }
    // Burada sol ve sağ operandların tipleri kontrol edilmeli ve operatörle uyumlu olup olmadığına bakılmalıdır.
    // Örnek olarak basit bir kontrol yapılmıyor.
    analyze_node(node->children[0], context);
    analyze_node(node->children[1], context);
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result analyze_identifier(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    if (node->token != NULL) {
        c_ampersand_symbol_entry *symbol = c_ampersand_semantics_lookup_symbol(context, node->token->lexeme);
        if (symbol == NULL) {
            c_ampersand_semantics_report_error(node, "Tanımsız değişken '%s'.", node->token->lexeme);
            return (c_ampersand_result){C_AMPERSAND_ERROR_SEMANTIC};
        }
        // Sembol bulundu, tipi kullanılabilir.
    }
    return C_AMPERSAND_OK_RESULT;
}

static c_ampersand_result analyze_node(c_ampersand_ast_node *node, c_ampersand_semantic_context *context) {
    if (node == NULL) {
        return C_AMPERSAND_OK_RESULT;
    }

    switch (node->type) {
        case AST_NODE_PROGRAM:
            return analyze_program(node, context);
        case AST_NODE_VARIABLE_DECLARATION:
            return analyze_variable_declaration(node, context);
        case AST_NODE_BINARY_EXPRESSION:
            return analyze_binary_expression(node, context);
        case AST_NODE_IDENTIFIER:
            return analyze_identifier(node, context);
        // ... diğer düğüm tipleri için analiz fonksiyonları eklenebilir
        default:
            // Bilinmeyen düğüm tipi
            break;
    }

    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_semantics_analyze(c_ampersand_ast_node *root) {
    c_ampersand_println("Semantik analiz başlatılıyor...");
    c_ampersand_symbol_table *global_scope = c_ampersand_semantics_create_symbol_table(NULL);
    c_ampersand_semantic_context context = { .current_scope = global_scope };
    c_ampersand_result result = analyze_node(root, &context);
    // Sembol tablosunu serbest bırakma (basit örnekte atlanmıştır)
    c_ampersand_println("Semantik analiz tamamlandı.");
    return result;
}

// ... diğer semantik analiz ile ilgili fonksiyonların implementasyonları buraya gelebilir
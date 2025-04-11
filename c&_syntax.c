#include "c&_syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- Token Dizileri (Örnek) ---
const char *c_ampersand_keyword_strings[] = {
    "func", "let", "if", "else", "while", "return", NULL
};

const c_ampersand_token_type c_ampersand_keyword_types[] = {
    TOKEN_KEYWORD_FUNC, TOKEN_KEYWORD_LET, TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE, TOKEN_KEYWORD_WHILE, TOKEN_KEYWORD_RETURN
};

const char *c_ampersand_operator_strings[] = {
    "+", "-", "*", "/", "=", "==", "!=", "<", ">", NULL
};

const c_ampersand_token_type c_ampersand_operator_types[] = {
    TOKEN_OPERATOR_PLUS, TOKEN_OPERATOR_MINUS, TOKEN_OPERATOR_MULTIPLY,
    TOKEN_OPERATOR_DIVIDE, TOKEN_OPERATOR_ASSIGN, TOKEN_OPERATOR_EQUAL,
    TOKEN_OPERATOR_NOT_EQUAL, TOKEN_OPERATOR_LESS_THAN, TOKEN_OPERATOR_GREATER_THAN
};

const char *c_ampersand_punctuation_strings[] = {
    "(", ")", "{", "}", "[", "]", ",", ";", ":", NULL
};

const c_ampersand_token_type c_ampersand_punctuation_types[] = {
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET, TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_COLON
};

// --- Token ve AST Düğümü Oluşturma Fonksiyonları ---

c_ampersand_token *c_ampersand_syntax_create_token(c_ampersand_token_type type, const char *lexeme, int line, int column) {
    c_ampersand_token *token = malloc(sizeof(c_ampersand_token));
    if (token == NULL) {
        return NULL;
    }
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;
    return token;
}

c_ampersand_ast_node *c_ampersand_syntax_create_ast_node(c_ampersand_ast_node_type type, c_ampersand_token *token) {
    c_ampersand_ast_node *node = malloc(sizeof(c_ampersand_ast_node));
    if (node == NULL) {
        return NULL;
    }
    node->type = type;
    node->children = NULL;
    node->num_children = 0;
    node->children_capacity = 0;
    node->token = token;
    return node;
}

c_ampersand_result c_ampersand_syntax_add_child(c_ampersand_ast_node *parent, c_ampersand_ast_node *child) {
    if (parent == NULL || child == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    if (parent->num_children >= parent->children_capacity) {
        parent->children_capacity = (parent->children_capacity == 0) ? 4 : parent->children_capacity * 2;
        c_ampersand_ast_node **new_children = realloc(parent->children, sizeof(c_ampersand_ast_node *) * parent->children_capacity);
        if (new_children == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        parent->children = new_children;
    }
    parent->children[parent->num_children++] = child;
    return C_AMPERSAND_OK_RESULT;
}

void c_ampersand_syntax_free_ast_node(c_ampersand_ast_node *node) {
    if (node == NULL) {
        return;
    }
    for (size_t i = 0; i < node->num_children; ++i) {
        c_ampersand_syntax_free_ast_node(node->children[i]);
    }
    free(node->children);
    if (node->token != NULL) {
        c_ampersand_syntax_free_token(node->token);
    }
    free(node);
}

void c_ampersand_syntax_free_token(c_ampersand_token *token) {
    if (token == NULL) {
        return;
    }
    free(token->lexeme);
    free(token);
}

// ... diğer sözdizimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
#include "c&_syntax.h"
// Temel C& tipler ve hata yönetimi için
#include "c&_stdlib.h"
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"

#include <string.h> // strlen, strcmp, memcpy için
#include <stdbool.h> // bool için


// --- Token Dizileri (Lexer tarafından kullanılabilir) ---
// Const olduğu için dinamik bellek yönetimi gerektirmezler.
// Bu verilere lexer modülü c&_syntax.h'yi include ederek erişebilir.
const char *c_ampersand_keyword_strings[] = {
    "func", "let", "if", "else", "while", "return", "true", "false", NULL // true/false eklendi
};

const c_ampersand_token_type c_ampersand_keyword_types[] = {
    TOKEN_KEYWORD_FUNC, TOKEN_KEYWORD_LET, TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE, TOKEN_KEYWORD_WHILE, TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_TRUE, TOKEN_KEYWORD_FALSE
};

const char *c_ampersand_operator_strings[] = {
    "+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "!", NULL // <=, >=, &&, ||, ! eklendi
};

const c_ampersand_token_type c_ampersand_operator_types[] = {
    TOKEN_OPERATOR_PLUS, TOKEN_OPERATOR_MINUS, TOKEN_OPERATOR_MULTIPLY,
    TOKEN_OPERATOR_DIVIDE, TOKEN_OPERATOR_ASSIGN, TOKEN_OPERATOR_EQUAL,
    TOKEN_OPERATOR_NOT_EQUAL, TOKEN_OPERATOR_LESS_THAN, TOKEN_OPERATOR_GREATER_THAN,
    TOKEN_OPERATOR_LESS_EQUAL, TOKEN_OPERATOR_GREATER_EQUAL,
    TOKEN_OPERATOR_AND, TOKEN_OPERATOR_OR, TOKEN_OPERATOR_NOT
};

const char *c_ampersand_punctuation_strings[] = {
    "(", ")", "{", "}", "[", "]", ",", ";", ":", "->", ".", NULL // ->, . eklendi
};

const c_ampersand_token_type c_ampersand_punctuation_types[] = {
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET, TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_COLON,
    TOKEN_ARROW, TOKEN_DOT
};


// --- Token ve AST Düğümü Oluşturma Fonksiyonları ---

// Yeni bir token oluşturur. Lexeme stringini C& belleğinde tahsis eder.
c_ampersand_token *c_ampersand_syntax_create_token(c_ampersand_token_type type, const char *lexeme, int line, int column) {
    // Parametre doğrulama
    if (lexeme == NULL) {
         // Lexeme NULL ise, bu bir programlama hatasıdır veya EOF/Comment gibi tokenlar için özel durum olmalıdır.
         // Geçersiz argüman hatası logla ve NULL dön.
         c_ampersand_println("SYNTAX HATA: create_token NULL lexeme ile çağrıldı (Tip: %d).", type);
          c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "create_token: NULL lexeme.").message; // Bu fonksiyon pointer dönüyor
         return NULL; // Hata durumunda NULL pointer dönmek uygun
    }

    c_ampersand_token *token = NULL;
    size_t token_struct_size = sizeof(c_ampersand_token);
    // Token structı için bellek tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(token_struct_size, (void**)&token);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği, hata c_ampersand_allocate içinde raporlanır.
        return NULL; // NULL döndür
    }

    // Tahsis edilen belleği sıfırla
    memset(token, 0, token_struct_size);

    // Lexeme stringi için bellek tahsis et ve kopyala
    size_t lexeme_len = strlen(lexeme);
    size_t lexeme_alloc_size = lexeme_len + 1; // Null sonlandırıcı dahil
    char *lexeme_ptr = NULL;
    alloc_res = c_ampersand_allocate(lexeme_alloc_size, (void**)&lexeme_ptr);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Lexeme tahsis hatası, token structını serbest bırak
        c_ampersand_free(token, token_struct_size); // Token structını free et
        return NULL; // NULL döndür
    }

    // Lexeme stringini kopyala
    memcpy(lexeme_ptr, lexeme, lexeme_alloc_size);

    // Token alanlarını doldur
    token->type = type;
    token->lexeme = lexeme_ptr;
    token->lexeme_allocated_size = lexeme_alloc_size; // Lexeme boyutu
    token->line = line;
    token->column = column;
    token->token_allocated_size = token_struct_size; // Token struct boyutu

    return token; // Başarı
}

// Yeni bir AST düğümü oluşturur. Düğüm struct'ını C& belleğinde tahsis eder.
c_ampersand_ast_node *c_ampersand_syntax_create_ast_node(c_ampersand_ast_node_type type, c_ampersand_token *token) {
    c_ampersand_ast_node *node = NULL;
    size_t node_struct_size = sizeof(c_ampersand_ast_node);
    // AST düğümü structı için bellek tahsis et
    c_ampersand_result alloc_res = c_ampersand_allocate(node_struct_size, (void**)&node);

    if (alloc_res.code != C_AMPERSAND_OK) {
        // Bellek yetersizliği, hata c_ampersand_allocate içinde raporlanır.
        return NULL; // NULL döndür
    }

    // Tahsis edilen belleği sıfırla
    memset(node, 0, node_struct_size);

    // Düğüm alanlarını doldur
    node->type = type;
    node->children = NULL;
    node->num_children = 0;
    node->children_capacity = 0;
    node->children_allocated_size = 0; // Başlangıçta children dizisi yok
    node->token = token; // Token pointerını ata (belleği ayrı yönetilir)
    node->node_allocated_size = node_struct_size; // Düğüm struct boyutu

    return node; // Başarı
}

// Bir AST düğümüne alt düğüm ekler. Alt düğüm dizisini dinamik olarak genişletebilir.
c_ampersand_result c_ampersand_syntax_add_child(c_ampersand_ast_node *parent, c_ampersand_ast_node *child) {
    // Parametre doğrulama
    if (parent == NULL || child == NULL) {
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_INVALID_ARGUMENT, "add_child: Geçersiz argüman (NULL parent veya child).");
    }

    // Kapasite yeterli değilse diziyi genişlet
    if (parent->num_children >= parent->children_capacity) {
        size_t new_capacity = (parent->children_capacity == 0) ? 4 : parent->children_capacity * 2; // Yeni kapasite hesapla
        size_t old_allocated_size = parent->children_allocated_size;
        size_t new_allocated_size = new_capacity * sizeof(c_ampersand_ast_node *);

        c_ampersand_ast_node **new_children = NULL;
        c_ampersand_result realloc_res;

        if (parent->children == NULL) {
            // İlk kez bellek tahsisi
             realloc_res = c_ampersand_allocate(new_allocated_size, (void**)&new_children);
        } else {
            // Mevcut diziyi genişlet
            realloc_res = c_ampersand_reallocate(parent->children, old_allocated_size, new_allocated_size, (void**)&new_children);
        }

        if (realloc_res.code != C_AMPERSAND_OK) {
            // Bellek yetersizliği, hata c_ampersand_reallocate/allocate içinde raporlanır.
            return realloc_res; // Bellek hatasını ilet (mesajı allocate/reallocate içinde gelir)
        }

        // Başarılı reallocate/allocate sonrası parent pointerlarını güncelle
        parent->children = new_children;
        parent->children_capacity = new_capacity;
        parent->children_allocated_size = new_allocated_size; // Yeni tahsis edilen boyutu kaydet

        // Not: realloc yeni tahsis edilen alanı sıfırlamayabilir.
        // Eğer kapasite 0'dan new_capacity'ye geçtiyse, yeni alanı sıfırlamak gerekebilir.
        // Ancak pointer dizisi olduğu için buna genellikle gerek duyulmaz, boş alanlar kullanılmayacaktır.
    }

    // Alt düğümü diziye ekle
    parent->children[parent->num_children++] = child;

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir AST düğümünü ve altındaki tüm düğümleri rekürsif olarak serbest bırakır.
// c_ampersand_free kullanır. Hataları loglar.
void c_ampersand_syntax_free_ast_node(c_ampersand_ast_node *node) {
    if (node == NULL) {
        return; // NULL pointer güvenliği
    }

    // Alt düğümleri rekürsif olarak serbest bırak
    for (size_t i = 0; i < node->num_children; ++i) {
        c_ampersand_syntax_free_ast_node(node->children[i]);
    }

    // Alt düğümler dizisini serbest bırak
    if (node->children != NULL && node->children_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(node->children, node->children_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
            c_ampersand_println("SYNTAX UYARI: AST düğümü çocuk dizisi serbest bırakılırken hata (%d).", free_res.code);
              c_ampersand_result_print_error(free_res); // Detaylı hatayı yazdır
              c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
        node->children = NULL;
        node->children_allocated_size = 0;
        node->num_children = 0;
        node->children_capacity = 0;
    } else if (node->children != NULL && node->children_allocated_size == 0) {
        // Pointer var ama boyut 0? Potansiyel hata.
         c_ampersand_println("SYNTAX UYARI: AST düğümü çocuk dizisi pointerı NULL değil ama boyut 0.");
    }


    // İlişkili token'ı serbest bırak (token NULL olabilir)
    if (node->token != NULL) {
        c_ampersand_syntax_free_token(node->token); // free_token içindeki hatalar orada loglanır
    }
    node->token = NULL;


    // AST düğüm struct'ını serbest bırak
    if (node->node_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(node, node->node_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
            c_ampersand_println("SYNTAX UYARI: AST düğümü structı serbest bırakılırken hata (%d).", free_res.code);
              c_ampersand_result_print_error(free_res); // Detaylı hatayı yazdır
              c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
    } else {
         c_ampersand_println("SYNTAX UYARI: Free edilmek istenen AST düğümü structı allocated_size 0.");
    }
}

// Bir token'ı serbest bırakır. Lexeme stringini ve token struct'ını serbest bırakır.
// c_ampersand_free kullanır. Hataları loglar.
void c_ampersand_syntax_free_token(c_ampersand_token *token) {
    if (token == NULL) {
        return; // NULL pointer güvenliği
    }

    // Lexeme stringini serbest bırak
    if (token->lexeme != NULL && token->lexeme_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(token->lexeme, token->lexeme_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
             c_ampersand_println("SYNTAX UYARI: Token lexeme serbest bırakılırken hata (%d).", free_res.code);
              c_ampersand_result_print_error(free_res); // Detaylı hatayı yazdır
              c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
        token->lexeme = NULL;
        token->lexeme_allocated_size = 0;
    } else if (token->lexeme != NULL && token->lexeme_allocated_size == 0) {
         // Pointer var ama boyut 0? Potansiyel hata.
         c_ampersand_println("SYNTAX UYARI: Token lexeme pointerı NULL değil ama boyut 0.");
    }


    // Token struct'ını serbest bırak
    if (token->token_allocated_size > 0) {
        c_ampersand_result free_res = c_ampersand_free(token, token->token_allocated_size);
        if (free_res.code != C_AMPERSAND_OK) {
             c_ampersand_println("SYNTAX UYARI: Token structı serbest bırakılırken hata (%d).", free_res.code);
              c_ampersand_result_print_error(free_res); // Detaylı hatayı yazdır
              c_ampersand_result_free_message(&free_res); // Mesajı serbest bırak
        }
    } else {
         c_ampersand_println("SYNTAX UYARI: Free edilmek istenen token structı allocated_size 0.");
    }
}


// --- Lexer/Parser Yardımcı Verileri (String karşılıkları) ---
// Lexer veya parser tarafından kullanılabilir.
// extern olarak c&_syntax.h'de bildirilirse diğer modüller erişebilir.

extern const char *c_ampersand_keyword_strings[];
extern const c_ampersand_token_type c_ampersand_keyword_types[];
extern const char *c_ampersand_operator_strings[];
extern const c_ampersand_token_type c_ampersand_operator_types[];
extern const char *c_ampersand_punctuation_strings[];
extern const c_ampersand_token_type c_ampersand_punctuation_types[];

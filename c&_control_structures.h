#ifndef C_AMPERSAND_CONTROL_STRUCTURES_H
#define C_AMPERSAND_CONTROL_STRUCTURES_H

#include "c&_stdlib.h"     // Temel tipler için
#include "c&_syntax.h"       // AST düğüm tipleri için
#include "c&_data_types.h" // Değerleri tutmak için

// Bir ifadeyi değerlendirmek için kullanılacak fonksiyonun tip tanımı
typedef c_ampersand_result (*c_ampersand_evaluate_expression_func)(const c_ampersand_ast_node *expression_node, c_ampersand_value *result);

// Bir ifade listesini (örneğin, bir blok içindeki ifadeler) yürütmek için kullanılacak fonksiyonun tip tanımı
typedef c_ampersand_result (*c_ampersand_execute_statement_func)(const c_ampersand_ast_node *statement_node);

// "if" ifadesini yürütür.
c_ampersand_result c_ampersand_control_execute_if(const c_ampersand_ast_node *if_node,
                                                 c_ampersand_evaluate_expression_func evaluate_expression,
                                                 c_ampersand_execute_statement_func execute_statement);

// "while" ifadesini yürütür.
c_ampersand_result c_ampersand_control_execute_while(const c_ampersand_ast_node *while_node,
                                                    c_ampersand_evaluate_expression_func evaluate_expression,
                                                    c_ampersand_execute_statement_func execute_statement);

// "return" ifadesini yürütür.
c_ampersand_result c_ampersand_control_execute_return(const c_ampersand_ast_node *return_node,
                                                     c_ampersand_evaluate_expression_func evaluate_expression,
                                                     c_ampersand_value *return_value);

// Bir blok ifadesini (süslü parantezler içindeki ifadeler) yürütür.
c_ampersand_result c_ampersand_control_execute_block(const c_ampersand_ast_node *block_node,
                                                    c_ampersand_execute_statement_func execute_statement);

// ... diğer kontrol yapıları (for, vb.) için fonksiyon prototipleri eklenebilir

#endif // C_AMPERSAND_CONTROL_STRUCTURES_H
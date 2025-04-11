#include "c&_variables.h"
#include "c&_stdlib.h"
#include "c&_data_types.h"
#include "sahne64_api.h" // Gerekirse bellek yönetimi için
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Global scope tanımı
c_ampersand_scope *global_scope = NULL;

// --- Değişken Yönetimi ---

c_ampersand_variable *c_ampersand_variable_create(const char *name, c_ampersand_type type) {
    c_ampersand_variable *var = malloc(sizeof(c_ampersand_variable));
    if (var == NULL) {
        return NULL;
    }
    var->name = strdup(name);
    if (var->name == NULL) {
        free(var);
        return NULL;
    }
    var->type = type;
    c_ampersand_result result = c_ampersand_value_create(type, &var->value);
    if (result.code != C_AMPERSAND_OK) {
        free(var->name);
        free(var);
        return NULL;
    }
    var->next = NULL;
    return var;
}

void c_ampersand_variable_free(c_ampersand_variable *var) {
    if (var == NULL) {
        return;
    }
    free(var->name);
    c_ampersand_value_free(&var->value);
    free(var);
}

c_ampersand_result c_ampersand_variable_set_value(c_ampersand_variable *var, const c_ampersand_value *value) {
    if (var == NULL || value == NULL || var->type.kind != value->type.kind) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    // Burada daha detaylı tür kontrolü yapılabilir (örneğin, genişletme).
    // Basit bir örnek olarak doğrudan atama yapılıyor.
    var->value = *value;
    // Eğer değer pointer veya string ise, derinlemesine kopyalama gerekebilir.
    if (var->type.kind == TYPE_STRING && value->value.string_val != NULL) {
        var->value.value.string_val = strdup(value->value.string_val);
        if (var->value.value.string_val == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
    } else if (var->type.kind == TYPE_POINTER) {
        // Pointer'lar doğrudan kopyalanır (sığ kopyalama).
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_variable_get_value(const c_ampersand_variable *var, c_ampersand_value *out_value) {
    if (var == NULL || out_value == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    *out_value = var->value;
    // Eğer değer pointer veya string ise, derinlemesine kopyalama gerekebilir (isteğe bağlı).
    if (var->type.kind == TYPE_STRING && var->value.value.string_val != NULL) {
        out_value->value.string_val = strdup(var->value.value.string_val);
        if (out_value->value.string_val == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
    }
    return C_AMPERSAND_OK_RESULT;
}

// --- Scope Yönetimi ---

c_ampersand_scope *c_ampersand_scope_create(c_ampersand_scope *parent) {
    c_ampersand_scope *scope = malloc(sizeof(c_ampersand_scope));
    if (scope == NULL) {
        return NULL;
    }
    scope->variables = NULL;
    scope->parent = parent;
    return scope;
}

void c_ampersand_scope_free(c_ampersand_scope *scope) {
    if (scope == NULL) {
        return;
    }
    c_ampersand_variable *current = scope->variables;
    while (current != NULL) {
        c_ampersand_variable *next = current->next;
        c_ampersand_variable_free(current);
        current = next;
    }
    free(scope);
}

c_ampersand_result c_ampersand_scope_add_variable(c_ampersand_scope *scope, c_ampersand_variable *var) {
    if (scope == NULL || var == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }
    var->next = scope->variables;
    scope->variables = var;
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_variable *c_ampersand_variable_lookup(c_ampersand_scope *scope, const char *name) {
    c_ampersand_scope *current_scope = scope;
    while (current_scope != NULL) {
        c_ampersand_variable *current_var = current_scope->variables;
        while (current_var != NULL) {
            if (strcmp(current_var->name, name) == 0) {
                return current_var;
            }
            current_var = current_var->next;
        }
        current_scope = current_scope->parent;
    }
    return NULL; // Değişken bulunamadı
}

// --- Başlangıç ve Kapanış ---

c_ampersand_result c_ampersand_variables_init() {
    if (global_scope == NULL) {
        global_scope = c_ampersand_scope_create(NULL);
        if (global_scope == NULL) {
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        return C_AMPERSAND_OK_RESULT;
    }
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_variables_shutdown() {
    if (global_scope != NULL) {
        c_ampersand_scope_free(global_scope);
        global_scope = NULL;
    }
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer değişken ve scope yönetimi ile ilgili fonksiyonların implementasyonları buraya gelebilir
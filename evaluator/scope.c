#include "scope.h"

#include <string.h>

scope_t *make_scope(scope_t *outer) {
  scope_t *scope = (scope_t *)calloc(1, sizeof(scope_t));
  *scope = (scope_t){.outer = outer,
                     .cap = 4,
                     .cnt = 0,
                     .table = (value_t **)calloc(4, sizeof(value_t *))};
  return scope;
}

void destroy_scope(scope_t *scope) {
  for (size_t i = 0; i < scope->cnt; i++) {
    destroy_value(scope->table[i]);
  }
  free(scope->table);
  free(scope);
}

value_t *look_up_in(scope_t *scope, char *name) {
  if (scope == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < scope->cnt; i++) {
    if (strcmp(((variable_t *)scope->table[i])->name, name) == 0) {
      return scope->table[i];
    }
  }
  return look_up_in(scope->outer, name);
}

void add_symbol(scope_t *scope, value_t *var) {
  if (scope->cnt >= scope->cap) {
    scope->cap *= 2;
    scope->table =
        (value_t **)realloc(scope->table, scope->cap * sizeof(value_t *));
  }
  scope->table[scope->cnt++] = var;
}
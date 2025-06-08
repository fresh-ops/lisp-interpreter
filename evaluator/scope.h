#pragma once

#include "types.h"

typedef struct scope {
  struct scope *outer;
  struct scope *closure;
  size_t cap;
  size_t cnt;
  value_t **table;
} scope_t;

scope_t *make_scope(scope_t *outer);

void destroy_scope(scope_t *scope);

void destroy_scope_rec(scope_t *scope);

scope_t *copy_scope(scope_t *scope);

/**
 * @brief Searchs for name in the passed scope and the outer one
 *
 * @param scope The scope to search in
 * @param name The identifier to search
 * @return value_t* Corresponding value or NULL if there is no such
 */
value_t *look_up_in(scope_t *scope, char *name);

/**
 * @brief Adds symbol to the provided scope
 */
void add_symbol(scope_t *scope, value_t *var);

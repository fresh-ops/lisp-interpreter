#pragma once

#include <stdlib.h>

#include "scope.h"
#include "types.h"

void set_scope(scope_t *scope);

scope_t *get_scope();

value_t *head(value_t **args);

value_t *tail(value_t **args);

value_t *cat(value_t **args);

value_t *list(value_t **args);

value_t *isnil(value_t **args);

value_t *del(value_t **args);

value_t *sum(value_t **args);

value_t *sub(value_t **args);

value_t *gt(value_t **args);

value_t *lt(value_t **args);

value_t *ge(value_t **args);

value_t *le(value_t **args);

value_t *eq(value_t **args);

value_t *lor(value_t **args);

value_t *land(value_t **args);

value_t *lnot(value_t **args);

value_t *funcall(value_t **args);

value_t *mapcar(value_t **args);
#pragma once

#include <stdlib.h>

#include "scope.h"
#include "types.h"

void set_scope(scope_t *scope);

scope_t *get_scope();

value_t *sum(value_t **args);

value_t *sub(value_t **args);

value_t *gt(value_t **args);

value_t *lt(value_t **args);

value_t *ge(value_t **args);

value_t *le(value_t **args);

value_t *funcall(value_t **args);

value_t *mapcar(value_t **args);
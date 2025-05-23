#pragma once

#include <stdlib.h>

#include "astree.h"

enum type { INT, STR, VAR, CORE, FUNC };

typedef struct value {
  enum type type;
} value_t;

typedef struct integer {
  enum type type;
  long long value;
} integer_t;

typedef struct string {
  enum type type;
  size_t length;
  char *value;
} string_t;

typedef struct variable {
  enum type type;
  char *name;
  value_t *data;
} variable_t;

typedef struct core_function {
  enum type type;
  char *name;
  size_t args_cnt;
  value_t *(*body)(value_t **);
} core_function_t;

typedef struct function {
  enum type type;
  char *name;
  size_t args_cnt;
  char **args;
  as_tree_t *body;
} function_t;

void destroy_value(value_t *value);
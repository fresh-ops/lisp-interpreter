#pragma once

#include <stdlib.h>

enum type { INT, STR, VAR };

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

void destroy_value(value_t *value);
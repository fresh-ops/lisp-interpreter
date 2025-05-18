#pragma once

#include <stdlib.h>

enum type { INT, STR };

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
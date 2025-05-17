#include <stdlib.h>

enum type { INT, STR };

typedef struct value {
  enum type t;
} value_t;

typedef struct integer {
  enum type t;
  long long value;
} integer_t;

typedef struct string {
  enum type t;
  size_t length;
  char *value;
} string_t;
#pragma once

#include "../parser/lexer.h"

enum construction { EXPRESSION, VALUE, NAME };

typedef struct as_tree {
  enum construction type;
  token_t *token;
  size_t cnt;
  size_t cap;
  size_t length;
  struct as_tree *children;
} as_tree_t;
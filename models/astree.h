#pragma once

#include "lexer.h"

enum construction { EXPRESSION, VALUE, NAME };

typedef struct as_tree {
  enum construction type;
  token_t *token;
  size_t cnt;
  size_t cap;
  size_t length;
  struct as_tree *children;
} as_tree_t;

/**
 * @brief Creates a deep copy of the passed tree
 */
as_tree_t *copy_tree(const as_tree_t *tree);

/**
 * @brief Destroys the passed tree with all children
 *
 * @param free_tree Pass 1 to free trees memory
 * @param free_token_text Pass 1 to free token text
 */
void destroy_tree(as_tree_t *tree, int free_tree, int free_token_text);
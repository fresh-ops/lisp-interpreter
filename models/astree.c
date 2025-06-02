#include "astree.h"

#include <stdlib.h>
#include <string.h>

as_tree_t *copy_tree(const as_tree_t *tree) {
  if (tree == NULL) return NULL;

  as_tree_t *copy = (as_tree_t *)malloc(sizeof(as_tree_t));
  memcpy(copy, tree, sizeof(as_tree_t));
  copy->children = NULL;

  if (tree->token != NULL) {
    copy->token = (token_t *)calloc(1, sizeof(token_t));
    copy->token->type = tree->token->type;
    copy->token->length = tree->token->length;
    copy->token->start = strndup(tree->token->start, tree->token->length);
  } else {
    copy->token = NULL;
  }

  if (tree->cap > 0 && tree->cnt > 0) {
    copy->children = (as_tree_t *)calloc(tree->cap, sizeof(as_tree_t));
    for (size_t i = 0; i < tree->cnt; i++) {
      as_tree_t *child = copy_tree(&tree->children[i]);
      memcpy(&copy->children[i], child, sizeof(as_tree_t));
      free(child);
    }
  }

  return copy;
}

void destroy_tree(as_tree_t *tree, int free_tree, int free_token_text) {
  if (tree == NULL) {
    return;
  }
  for (size_t i = 0; i < tree->cnt; i++) {
    destroy_tree(tree->children + i, 0, free_token_text);
  }
  free(tree->children);
  if (tree->token != NULL) {
    if (free_token_text) {
      free((void *)tree->token->start);
    }
    free(tree->token);
  }
  if (free_tree) {
    free(tree);
  }
}
#include "parser.h"

#include <string.h>

#include "lexer.h"

static as_tree_t *parse_expression(const char *input);

static void destroy_tree(as_tree_t *tree) {
  if (tree == NULL) {
    return;
  }
  free(tree->children);
  free(tree);
}

static size_t skip_whitespaces(const char *input) {
  const char *start = input;
  while (*input == ' ') {
    input++;
  }
  return input - start;
}

as_tree_t *parse(const char *input) {
  if (*input == '(') {
    return parse_expression(input);
  }

  token_t *token = parse_atom(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
  tree->token = token;
  tree->length = token->length;
  tree->type = (token->type == TOKEN_ID) ? NAME : VALUE;

  return tree;
}

static as_tree_t *parse_expression(const char *input) {
  const char *start = input++;

  token_t *token = parse_identifier(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
  *tree = (as_tree_t){.type = EXPRESSION,
                      .token = token,
                      .cap = 2,
                      .cnt = 0,
                      .children = (as_tree_t *)calloc(2, sizeof(as_tree_t))};

  input += token->length;
  input += skip_whitespaces(input);

  while (*input != ')' && *input != '\0') {
    if (tree->cnt >= tree->cap) {
      tree->cap *= 2;
      tree->children =
          (as_tree_t *)realloc(tree->children, tree->cap * sizeof(as_tree_t));
    }

    as_tree_t *child = parse(input);
    if (child == NULL) {
      destroy_tree(tree);
      return NULL;
    }

    memcpy(tree->children + tree->cnt++, child, sizeof(as_tree_t));
    input += child->length;
    input += skip_whitespaces(input);
    free(child);
  }
  if (*input != ')') {
    destroy_tree(tree);
    return NULL;
  }
  tree->length = input - start + 1;
  return tree;
}
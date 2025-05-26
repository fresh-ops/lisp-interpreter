#include "parser.h"

#include <string.h>

#include "lexer.h"

static as_tree_t *parse_expression(const char *input);

static size_t skip_whitespaces(const char *input) {
  const char *start = input;
  while (*input == ' ') {
    input++;
  }
  return input - start;
}

static as_tree_t *parse_symbol(const char *input, int quoted) {
  if (*input == '(') {
    const char *start = input++ - 1;
    as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
    *tree = (as_tree_t){.type = QUOTED,
                        .cap = 4,
                        .cnt = 0,
                        .children = (as_tree_t *)calloc(4, sizeof(as_tree_t))};
    while (*input != ')' && *input != '\0') {
      as_tree_t *child = parse_symbol(input, quoted);
      if (child == NULL) {
        destroy_tree(tree, 1, 0);
        return NULL;
      }
      child->length--;
      memcpy(&tree->children[tree->cnt++], child, sizeof(as_tree_t));
      input += child->length;
      input += skip_whitespaces(input);
      free(child);
    }
    if (*input != ')') {
      destroy_tree(tree, 1, 0);
      return NULL;
    }
    tree->length = input - start + 1;
    return tree;
  }
  token_t *token = parse_atom(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));

  if (tree == NULL) {
    free(token);
    return NULL;
  }

  tree->token = token;
  tree->length = (token->type == TOKEN_STR)
                     ? token->length + 2  // длинна строки не учитывает ковычки
                     : token->length;
  tree->type = (token->type == TOKEN_ID) ? NAME : VALUE;
  if (quoted) {
    tree->type = QUOTED;
    tree->length++;
  }

  return tree;
}

as_tree_t *parse(const char *input) {
  if (*input == '(') {
    return parse_expression(input);
  }
  int quoted = 0;
  if (*input == '\'') {
    input++;
    quoted = 1;
  }
  return parse_symbol(input, quoted);
}

static as_tree_t *parse_expression(const char *input) {
  const char *start = input++;

  token_t *token = parse_identifier(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
  if (tree == NULL) {
    free(token);
    return NULL;
  }
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
      destroy_tree(tree, 1, 0);
      return NULL;
    }

    memcpy(tree->children + tree->cnt++, child, sizeof(as_tree_t));
    input += child->length;
    input += skip_whitespaces(input);
    free(child);
  }
  if (*input != ')') {
    destroy_tree(tree, 1, 0);
    return NULL;
  }
  tree->length = input - start + 1;
  return tree;
}
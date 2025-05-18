#include "evaluator.h"
#include "../parser/parser.h"

#include <string.h>

static integer_t *extract_integer(const as_tree_t *tree) {
  char *value = extract_token(tree->token);
  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = (integer_t){.type = INT, .value = atoll(value)};
  free(value);
  return result;
}

static string_t *extract_string(const as_tree_t *tree) {
  string_t *result = (string_t *)calloc(1, sizeof(string_t));
  *result = (string_t){
      .type = STR, .length = tree->length, .value = extract_token(tree->token)};
  return result;
}

static value_t *evaluate_value(const as_tree_t *tree) {
    switch (tree->token->type)
    {
    case TOKEN_INT:
      return (value_t *)extract_integer(tree);
    case TOKEN_STR:
      return (value_t *)extract_string(tree);
    }
    return NULL;
}

value_t *evaluate_name(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  value_t *result = look_up_in(scope, name);
  free(name);
  return result;
}

value_t *evaluate(const as_tree_t *tree, scope_t *scope) {
    if (tree == NULL) {
      return NULL;
    }
    if (tree->type == VALUE) {
      return evaluate_value(tree);
    }
    if (tree->type == NAME) {
      return evaluate_name(tree, scope);
    }
    return NULL;
}
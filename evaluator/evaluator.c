#include "evaluator.h"

#include <string.h>

#include "../parser/parser.h"

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
  switch (tree->token->type) {
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
  if (result == NULL) {
    return NULL;
  }
  if (result->type != VAR) {
    return NULL;
  }
  return ((variable_t *)result)->data;
}

value_t *evaluate_core_function(core_function_t *func, const as_tree_t *tree,
                                scope_t *scope) {
  value_t **args = (value_t **)calloc(tree->cnt + 1, sizeof(value_t *));
  for (size_t i = 0; i < tree->cnt; i++) {
    args[i] = evaluate(&tree->children[i], scope);
    if (args[i] == NULL) {
      free(args);
      return NULL;
    }
  }
  value_t *result = func->body(args);
  free(args);
  return result;
}

void create_variable(const as_tree_t *tree, scope_t *scope) {
  variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
  *var = (variable_t){.type = VAR,
                      .name = extract_token(tree->children[0].token),
                      .data = evaluate(&tree->children[1], scope)};
  add_symbol(scope, (value_t *)var);
}

as_tree_t *copy_tree(as_tree_t *tree) {
  if (tree == NULL) return NULL;

  as_tree_t *copy = (as_tree_t *)calloc(1, sizeof(as_tree_t));
  memcpy(copy, tree, sizeof(as_tree_t));

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

void create_function(const as_tree_t *tree, scope_t *scope) {
  function_t *func = (function_t *)calloc(1, sizeof(function_t));
  *func = (function_t){
      .type = FUNC,
      .name = extract_token(tree->children[0].token),
      .args_cnt = tree->children[1].cnt + 1,
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .body = copy_tree(&tree->children[2])};
  func->args[0] = extract_token(tree->children[1].token);
  for (size_t i = 1; i < func->args_cnt; i++) {
    func->args[i] = extract_token(tree->children[1].children[i - 1].token);
  }
  add_symbol(scope, (value_t *)func);
}

value_t *evaluate_function(function_t *func, const as_tree_t *tree,
                           scope_t *scope) {
  scope_t *inner = make_scope(scope);
  for (size_t i = 0; i < func->args_cnt; i++) {
    variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
    *var = (variable_t){.type = VAR,
                        .name = func->args[i],
                        .data = evaluate(&tree->children[i], scope)};
    add_symbol(inner, (value_t *)var);
  }
  return evaluate(func->body, inner);
}

value_t *evaluate_expression(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  if (strcmp(name, "defun") == 0) {
    free(name);
    create_function(tree, scope);
    return NULL;
  } else if (strcmp(name, "defvar") == 0) {
    free(name);
    create_variable(tree, scope);
    return NULL;
  }
  value_t *symbol = look_up_in(scope, name);
  free(name);
  if (symbol->type == CORE) {
    return evaluate_core_function((core_function_t *)symbol, tree, scope);
  }
  if (symbol->type == FUNC) {
    return evaluate_function((function_t *)symbol, tree, scope);
  }
  return NULL;
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
  if (tree->type == EXPRESSION) {
    return evaluate_expression(tree, scope);
  }
  return NULL;
}
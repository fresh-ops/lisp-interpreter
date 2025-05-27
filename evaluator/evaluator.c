#include "evaluator.h"

#include <stdio.h>
#include <string.h>

#include "core.h"
#include "parser.h"

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
    default:
      break;
  }
  return NULL;
}

static value_t *evaluate_name(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  value_t *result = look_up_in(scope, name);
  free(name);
  if (result == NULL) {
    return NULL;
  }
  if (result->type != VAR) {
    return NULL;
  }
  return copy_value(((variable_t *)result)->data);
}

static value_t *evaluate_core_function(core_function_t *func,
                                       const as_tree_t *tree, scope_t *scope) {
  value_t **args = (value_t **)calloc(tree->cnt + 1, sizeof(value_t *));
  for (size_t i = 0; i < tree->cnt; i++) {
    args[i] = evaluate(&tree->children[i], scope);
    if (args[i] == NULL) {
      for (size_t j = 0; j < i; i++) {
        free(args[j]);
      }
      free(args);
      return NULL;
    }
  }

  set_scope(scope);
  value_t *result = func->body(args);
  set_scope(NULL);
  for (size_t i = 0; i < tree->cnt + 1; i++) {
    free(args[i]);
  }
  free(args);
  return result;
}

static void create_variable(const as_tree_t *tree, scope_t *scope) {
  variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
  *var = (variable_t){.type = VAR,
                      .name = extract_token(tree->children[0].token),
                      .data = evaluate(&tree->children[1], scope)};
  add_symbol(scope, (value_t *)var);
}

static value_t *create_function(const as_tree_t *tree, scope_t *scope) {
  function_t *func = (function_t *)calloc(1, sizeof(function_t));
  *func = (function_t){
      .type = FUNC,
      .name = extract_token(tree->children[0].token),
      .args_cnt = tree->children[1].cnt + 1,
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[2])};
  function_t *symbol = (function_t *)calloc(1, sizeof(function_t));
  *symbol = (function_t){
      .type = FUNC,
      .name = extract_token(tree->children[0].token),
      .args_cnt = tree->children[1].cnt + 1,
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[2])};
  func->args[0] = extract_token(tree->children[1].token);
  symbol->args[0] = extract_token(tree->children[1].token);
  for (size_t i = 1; i < func->args_cnt; i++) {
    func->args[i] = extract_token(tree->children[1].children[i - 1].token);
    symbol->args[i] = extract_token(tree->children[1].children[i - 1].token);
  }
  add_symbol(scope, (value_t *)func);
  return symbol;
}

static value_t *create_lambda(const as_tree_t *tree, scope_t *scope) {
  function_t *func = (function_t *)calloc(1, sizeof(function_t));
  *func = (function_t){
      .type = FUNC,
      .args_cnt = tree->children[0].cnt + 1,
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[1])};
  func->args[0] = extract_token(tree->children[0].token);
  for (size_t i = 1; i < func->args_cnt; i++) {
    func->args[i] = extract_token(tree->children[0].children[i - 1].token);
  }
  return func;
}

static value_t *evaluate_function(function_t *func, const as_tree_t *tree,
                                  scope_t *scope) {
  scope_t *inner = make_scope(scope);
  inner->closure = func->closure;
  for (size_t i = 0; i < func->args_cnt; i++) {
    variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
    *var = (variable_t){.type = VAR,
                        .name = strndup(func->args[i], strlen(func->args[i])),
                        .data = evaluate(&tree->children[i], inner)};
    add_symbol(inner, (value_t *)var);
  }
  value_t *result = evaluate(func->body, inner);
  destroy_scope(inner);
  return result;
}

static value_t *evaluate_expression(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  if (strcmp(name, "defun") == 0) {
    free(name);
    return create_function(tree, scope);
  } else if (strcmp(name, "defvar") == 0) {
    free(name);
    create_variable(tree, scope);
    return NULL;
  } else if (strcmp(name, "lambda") == 0) {
    free(name);
    return create_lambda(tree, scope);
  }
  value_t *symbol = look_up_in(scope, name);
  if (symbol == NULL) {
    printf("No such symbol %s\n", name);
    return NULL;
  }
  free(name);
  if (symbol->type == CORE) {
    return evaluate_core_function((core_function_t *)symbol, tree, scope);
  }
  if (symbol->type == FUNC) {
    return evaluate_function((function_t *)symbol, tree, scope);
  }
  return NULL;
}

static value_t *evaluate_quoted(const as_tree_t *tree, scope_t *scope);

static value_t *evaluate_list(const as_tree_t *tree, scope_t *scope) {
  list_t *start = (list_t *)calloc(1, sizeof(list_t));
  start->type = LIST;
  list_t *list = start;
  for (size_t i = 0; i < tree->cnt; i++) {
    list->data = evaluate_quoted(&tree->children[i], scope);
    if (i + 1 < tree->cnt) {
      list->next = (list_t *)calloc(1, sizeof(list_t));
      list = list->next;
      list->type = LIST;
    }
  }
  return (value_t *)start;
}

static value_t *evaluate_quoted(const as_tree_t *tree, scope_t *scope) {
  if (tree->token == NULL) {
    return evaluate_list(tree, scope);
  }
  switch (tree->token->type) {
    case TOKEN_INT:
      return (value_t *)extract_integer(tree);
    case TOKEN_STR:
      return (value_t *)extract_string(tree);
    case TOKEN_ID:
      char *name = extract_token(tree->token);
      // printf("%s\n", name);
      value_t *result = look_up_in(scope, name);
      free(name);
      return result;
    default:
      break;
  }
  return NULL;
}

value_t *evaluate_reference(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  value_t *symbol = look_up_in(scope, name);
  free(name);
  if (symbol == NULL) {
    return NULL;
  }
  switch (symbol->type) {
    case CORE:
      core_function_t *core =
          (core_function_t *)calloc(1, sizeof(core_function_t));
      memcpy(core, symbol, sizeof(core_function_t));
      core->name = strdup(core->name);
      return core;
    case FUNC:
      function_t *func = (function_t *)calloc(1, sizeof(function_t));
      memcpy(func, symbol, sizeof(function_t));
      func->name = strdup(((function_t *)symbol)->name);
      char **args = (char **)calloc(func->args_cnt, sizeof(char *));
      for (size_t i = 0; i < func->args_cnt; i++) {
        args[i] = strdup(func->args[i]);
      }
      func->args = args;
      func->body = copy_tree(((function_t *)symbol)->body);
      return func;
    default:
      return NULL;
  }
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
  if (tree->type == QUOTED) {
    return evaluate_quoted(tree, scope);
  }
  if (tree->type == REFERENCE) {
    return evaluate_reference(tree, scope);
  }
  return NULL;
}
#include "evaluator.h"

#include <stdio.h>
#include <string.h>

#include "cache.h"
#include "core.h"
#include "parser.h"

#define MAX_DEPTH 10000

static size_t depth = 0;

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
  fprintf(stderr, "Evaluator error: Unresolved value type\n");
  return NULL;
}

static value_t *evaluate_name(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  value_t *result = look_up_in(scope, name);
  if (result == NULL || result->type != VAR) {
    fprintf(stderr, "Evaluator error: variable %s has no value\n", name);
    free(name);
    return NULL;
  }
  return copy_value(((variable_t *)result)->data);
}

static value_t *evaluate_core_function(core_function_t *func,
                                       const as_tree_t *tree, scope_t *scope) {
  if (func->args_cnt > tree->cnt) {
    fprintf(stderr,
            "Evaluator error: Too few arguments passed to function %s\n",
            func->name);
    return NULL;
  }
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
  value_t *result = search_cache(func->name, args);
  if (result == NULL) {
    result = func->body(args);
    cache_result(func->name, args, result);
  }
  set_scope(NULL);
  for (size_t i = 0; i < tree->cnt + 1; i++) {
    free(args[i]);
  }
  free(args);
  return result;
}

static value_t *create_variable(const as_tree_t *tree, scope_t *scope) {
  if (tree->cnt < 2) {
    fprintf(stderr,
            "Evaluator error: too few parameters passed to operator defvar\n");
    return NULL;
  }
  if (tree->cnt > 2) {
    fprintf(stderr,
            "Evaluator error: too many parameters passed to operator defvar\n");
    return NULL;
  }
  if (tree->children[0].token->type != TOKEN_ID) {
    char *name = extract_token(tree->children[0].token);
    fprintf(stderr,
            "Evaluator error: \"%s\" is not a name of variable. Expected an "
            "identifier\n",
            name);
    free(name);
    return NULL;
  }

  variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
  *var = (variable_t){.type = VAR,
                      .name = extract_token(tree->children[0].token),
                      .data = evaluate(&tree->children[1], scope)};
  add_symbol(scope, (value_t *)var);
  var = (variable_t *)copy_value((value_t *)var);
  return (value_t *)var;
}

static value_t *create_function(const as_tree_t *tree, scope_t *scope) {
  if (tree->cnt < 3) {
    fprintf(stderr,
            "Evaluator error: too few parameters passed to operator defun\n");
    return NULL;
  }
  if (tree->cnt > 3) {
    fprintf(stderr,
            "Evaluator error: too many parameters passed to operator defun\n");
    return NULL;
  }
  if (tree->children[0].token->type != TOKEN_ID) {
    char *name = extract_token(tree->children[0].token);
    fprintf(stderr,
            "Evaluator error: \"%s\" is not a name of function. Expected an "
            "identifier\n",
            name);
    free(name);
    return NULL;
  }

  function_t *func = (function_t *)calloc(1, sizeof(function_t));
  *func = (function_t){
      .type = FUNC,
      .name = extract_token(tree->children[0].token),
      .args_cnt = tree->children[1].cnt + (tree->children[1].token != NULL),
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[2])};
  function_t *symbol = (function_t *)calloc(1, sizeof(function_t));
  *symbol = (function_t){
      .type = FUNC,
      .name = extract_token(tree->children[0].token),
      .args_cnt = tree->children[1].cnt + (tree->children[1].token != NULL),
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[2])};
  if (tree->children[1].token != NULL) {
    func->args[0] = extract_token(tree->children[1].token);
    symbol->args[0] = extract_token(tree->children[1].token);
  }
  for (size_t i = 1; i < func->args_cnt; i++) {
    func->args[i] = extract_token(tree->children[1].children[i - 1].token);
    symbol->args[i] = extract_token(tree->children[1].children[i - 1].token);
  }
  add_symbol(scope, (value_t *)func);
  return (value_t *)symbol;
}

static value_t *create_lambda(const as_tree_t *tree, scope_t *scope) {
  if (tree->cnt < 2) {
    fprintf(stderr,
            "Evaluator error: too few parameters passed to operator lambda\n");
    return NULL;
  }
  if (tree->cnt > 2) {
    fprintf(stderr,
            "Evaluator error: too many parameters passed to operator lambda\n");
    return NULL;
  }
  function_t *func = (function_t *)calloc(1, sizeof(function_t));
  *func = (function_t){
      .type = FUNC,
      .args_cnt = tree->children[0].cnt + (tree->children[0].token != NULL),
      .args = (char **)calloc(tree->children[1].cnt + 1, sizeof(char *)),
      .closure = (void *)copy_scope(scope),
      .body = copy_tree(&tree->children[1])};
  if (tree->children[1].token != NULL) {
    func->args[0] = extract_token(tree->children[0].token);
  }
  for (size_t i = 1; i < func->args_cnt; i++) {
    func->args[i] = extract_token(tree->children[0].children[i - 1].token);
  }
  return (value_t *)func;
}

static value_t *evaluate_function(function_t *func, const as_tree_t *tree,
                                  scope_t *scope) {
  if (func->args_cnt > tree->cnt) {
    fprintf(stderr,
            "Evaluator error: Too few arguments passed to function %s\n",
            func->name);
    return NULL;
  }
  if (func->args_cnt < tree->cnt) {
    fprintf(stderr,
            "Evaluator error: Too many arguments passed to function %s\n",
            func->name);
    return NULL;
  }

  scope_t *inner = make_scope(scope);
  inner->closure = func->closure;
  value_t **args = (value_t **)calloc(func->args_cnt + 1, sizeof(value_t *));
  for (size_t i = 0; i < func->args_cnt; i++) {
    variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
    *var = (variable_t){.type = VAR,
                        .name = strndup(func->args[i], strlen(func->args[i])),
                        .data = evaluate(&tree->children[i], inner)};
    args[i] = var->data;
    add_symbol(inner, (value_t *)var);
  }
  value_t *result = search_cache(func->name, args);
  if (result == NULL) {
    result = evaluate(func->body, inner);
    cache_result(func->name, args, result);
  }
  free(args);
  destroy_scope(inner);
  return result;
}

static value_t *evaluate_if(const as_tree_t *tree, scope_t *scope) {
  if (tree->cnt < 3) {
    fprintf(stderr,
            "Evaluator error: too few parameters passed to operator if\n");
    return NULL;
  }
  if (tree->cnt > 3) {
    fprintf(stderr,
            "Evaluator error: too many parameters passed to operator if\n");
    return NULL;
  }

  value_t *cond = evaluate(&tree->children[0], scope);
  if (is_nil(cond)) {
    destroy_value(cond);
    return evaluate(&tree->children[2], scope);
  }
  destroy_value(cond);
  return evaluate(&tree->children[1], scope);
}

static value_t *evaluate_expression(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  if (strcmp(name, "if") == 0) {
    free(name);
    return evaluate_if(tree, scope);
  }
  if (strcmp(name, "defun") == 0) {
    free(name);
    return create_function(tree, scope);
  } else if (strcmp(name, "defvar") == 0) {
    free(name);
    return create_variable(tree, scope);
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
      value_t *result = look_up_in(scope, name);
      free(name);
      return copy_value((value_t *)result);
    default:
      break;
  }
  return NULL;
}

value_t *evaluate_reference(const as_tree_t *tree, scope_t *scope) {
  char *name = extract_token(tree->token);
  value_t *symbol = look_up_in(scope, name);
  if (symbol == NULL || (symbol->type != CORE && symbol->type != FUNC)) {
    fprintf(stderr, "Evaluator error: Undefined function %s\n", name);
    free(name);
    return NULL;
  }
  free(name);
  switch (symbol->type) {
    case CORE:
      core_function_t *core =
          (core_function_t *)calloc(1, sizeof(core_function_t));
      memcpy(core, symbol, sizeof(core_function_t));
      core->name = strdup(core->name);
      return (value_t *)core;
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
      return (value_t *)func;
    default:
      return NULL;
  }
}

value_t *evaluate(const as_tree_t *tree, scope_t *scope) {
  if (tree == NULL) {
    return NULL;
  }
  depth++;
  if (depth > MAX_DEPTH) {
    depth = 0;
    fprintf(stderr, "Evaluator error: Exit maximum recurcion depth\n");
    return NULL;
  }
  value_t *result = NULL;
  if (tree->type == VALUE) {
    result = evaluate_value(tree);
  }
  if (tree->type == NAME) {
    result = evaluate_name(tree, scope);
  }
  if (tree->type == EXPRESSION) {
    result = evaluate_expression(tree, scope);
  }
  if (tree->type == QUOTED) {
    result = evaluate_quoted(tree, scope);
  }
  if (tree->type == REFERENCE) {
    result = evaluate_reference(tree, scope);
  }
  if (depth > 0) {
    depth--;
  }
  return result;
}
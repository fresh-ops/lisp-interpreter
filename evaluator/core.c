#include "core.h"

#include <stdio.h>

#include "evaluator.h"

static scope_t *_scope = NULL;

void set_scope(scope_t *scope) { _scope = scope; }

scope_t *get_scope() { return _scope; }

value_t *sum(value_t **args) {
  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = (integer_t){.type = INT, .value = 0};

  for (size_t i = 0; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      printf("Argument must be int\n");
      free(result);
      return NULL;
    }
    result->value += ((integer_t *)args[i])->value;
  }

  return (value_t *)result;
}

value_t *sub(value_t **args) {
  if (args[0]->type != INT) {
    return NULL;
  }

  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = *((integer_t *)args[0]);

  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      free(result);
      return NULL;
    }
    result->value -= ((integer_t *)args[i])->value;
  }

  return (value_t *)result;
}

value_t *funcall(value_t **args) {
  switch (args[0]->type) {
    case CORE:
      return ((core_function_t *)args[0])->body(args + 1);
    case FUNC:
      scope_t *outer = get_scope();
      scope_t *scope = make_scope(outer);
      function_t *func = (function_t *)args[0];
      scope->closure = func->closure;
      for (size_t i = 0; i < func->args_cnt; i++) {
        variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
        *var =
            (variable_t){.type = VAR,
                         .name = strndup(func->args[i], strlen(func->args[i])),
                         .data = args[i + 1]};
        args[i + 1] = NULL;
        add_symbol(scope, (value_t *)var);
      }
      value_t *result = evaluate(func->body, scope);
      destroy_scope(scope);
      return result;
    default:
      return NULL;
  }
}

value_t *mapcar(value_t **args) {
  if (args[1]->type != LIST) {
    return NULL;
  }
  list_t *list = args[1];
  list_t *list_start = list;
  list_t *result = (list_t *)calloc(1, sizeof(list_t));
  result->type = LIST;
  list_t *result_start = result;
  switch (args[0]->type) {
    case CORE:
      while (list != NULL) {
        args[1] = list->data;
        result->data = ((core_function_t *)args[0])->body(args + 1);
        if (list->next != NULL) {
          result->next = (list_t *)calloc(1, sizeof(list_t));
          result = result->next;
          result->type = LIST;
        }
        list = list->next;
      }
      args[1] = list_start;
      break;
    case FUNC:
      function_t *func = (function_t *)args[0];
      scope_t *scope = get_scope();
      while (list != NULL) {
        scope_t *inner = make_scope(scope);
        inner->closure = func->closure;
        variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
        *var =
            (variable_t){.type = VAR,
                         .name = strndup(func->args[0], strlen(func->args[0])),
                         .data = copy_value(list->data)};
        add_symbol(inner, (value_t *)var);
        result->data = evaluate(func->body, inner);
        destroy_scope(inner);
        if (list->next != NULL) {
          result->next = (list_t *)calloc(1, sizeof(list_t));
          result = result->next;
          result->type = LIST;
        }
        list = list->next;
      }
      args[1] = list_start;
      break;
    default:
      destroy_value(result);
      return NULL;
  }
  return (value_t *)result_start;
}
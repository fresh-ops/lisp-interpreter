#include "core.h"

#include <stdio.h>
#include <string.h>

#include "cache.h"
#include "evaluator.h"

static scope_t *_scope = NULL;

void set_scope(scope_t *scope) { _scope = scope; }

scope_t *get_scope() { return _scope; }

value_t *head(value_t **args) {
  if (args[0]->type != LIST) {
    fprintf(
        stderr,
        "Evaluator error: Type mismatch. The argument of function head must "
        "be list\n");
    return NULL;
  }
  return copy_value(((list_t *)args[0])->data);
}

value_t *tail(value_t **args) {
  if (args[0]->type != LIST) {
    fprintf(
        stderr,
        "Evaluator error: Type mismatch. The argument of function tail must "
        "be list\n");
    return NULL;
  }
  return copy_value((value_t *)((list_t *)args[0])->next);
}

value_t *cat(value_t **args) {
  if (args[0]->type != LIST || args[1]->type != LIST) {
    fprintf(
        stderr,
        "Evaluator error: Type mismatch. The arguments of function cat must "
        "be list\n");
    return NULL;
  }
  list_t *result = (list_t *)copy_value(args[0]);
  list_t *tail = result;
  while (tail->next != NULL) {
    tail = tail->next;
  }
  tail->next = (list_t *)copy_value(args[1]);
  return (value_t *)result;
}

value_t *list(value_t **args) {
  list_t *result = (list_t *)calloc(1, sizeof(list_t));
  result->type = LIST;
  list_t *cur = result;
  for (size_t i = 0; args[i] != NULL; i++) {
    cur->data = copy_value(args[i]);
    if (args[i + 1] != NULL) {
      cur->next = (list_t *)calloc(1, sizeof(list_t));
      cur = cur->next;
      cur->type = LIST;
    }
  }
  return (value_t *)result;
}

value_t *isnil(value_t **args) {
  if (args[1] != NULL) {
    fprintf(stderr,
            "Evaluator error: Too many arguments passed to function isnil\n");
    return NULL;
  }
  if (args[0]->type != LIST) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. The first argument of function "
            "isnil must be list\n");
    return NULL;
  }
  if (is_nil(args[0])) return make_true();
  return make_nil();
}

value_t *del(value_t **args) {
  if (args[0]->type != LIST) {
    fprintf(
        stderr,
        "Evaluator error: Type mismatch. The argument of function remove must "
        "be list\n");
    return NULL;
  }
  list_t *result = (list_t *)copy_value(args[0]);
  list_t *prev = result;
  list_t *cur = prev->next;
  if (compare_value(prev->data, args[1])) {
    prev->next = NULL;
    destroy_value((value_t *)prev);
    return (value_t *)cur;
  }
  while (cur != NULL) {
    if (compare_value(cur->data, args[1])) {
      prev->next = cur->next;
      cur->next = NULL;
      destroy_value((value_t *)cur);
      return (value_t *)result;
    }
    prev = cur;
    cur = cur->next;
  }
  return (value_t *)result;
}

value_t *sum(value_t **args) {
  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = (integer_t){.type = INT, .value = 0};

  for (size_t i = 0; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(stderr,
              "Evaluator error: Type mismatch. arguments of function sum must "
              "be integer\n");
      free(result);
      return NULL;
    }
    result->value += ((integer_t *)args[i])->value;
  }

  return (value_t *)result;
}

value_t *sub(value_t **args) {
  if (args[0]->type != INT) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. arguments of function sub must be "
            "integer\n");
    return NULL;
  }

  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = *((integer_t *)args[0]);

  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(stderr,
              "Evaluator error: Type mismatch. arguments of function sub must "
              "be integer\n");
      free(result);
      return NULL;
    }
    result->value -= ((integer_t *)args[i])->value;
  }

  return (value_t *)result;
}

value_t *gt(value_t **args) {
  if (args[0]->type != INT) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. arguments of function gt must be "
            "integer\n");
    return NULL;
  }
  integer_t *last = (integer_t *)args[0];
  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(
          stderr,
          "Evaluator error: Type mismatch. arguments of function gt must be "
          "integer\n");
      return NULL;
    }
    if (last->value <= ((integer_t *)args[i])->value) {
      return make_nil();
    }
    last = (integer_t *)args[i];
  }
  return make_true();
}

value_t *lt(value_t **args) {
  if (args[0]->type != INT) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. arguments of function lt must be "
            "integer\n");
    return NULL;
  }
  integer_t *last = (integer_t *)args[0];
  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(
          stderr,
          "Evaluator error: Type mismatch. arguments of function lt must be "
          "integer\n");
      return NULL;
    }
    if (last->value >= ((integer_t *)args[i])->value) {
      return make_nil();
    }
    last = (integer_t *)args[i];
  }
  return make_true();
}

value_t *ge(value_t **args) {
  if (args[0]->type != INT) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. arguments of function ge must be "
            "integer\n");
    return NULL;
  }
  integer_t *last = (integer_t *)args[0];
  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(
          stderr,
          "Evaluator error: Type mismatch. arguments of function ge must be "
          "integer\n");
      return NULL;
    }
    if (last->value < ((integer_t *)args[i])->value) {
      return make_nil();
    }
    last = (integer_t *)args[i];
  }
  return make_true();
}

value_t *le(value_t **args) {
  if (args[0]->type != INT) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. arguments of function le must be "
            "integer\n");
    return NULL;
  }
  integer_t *last = (integer_t *)args[0];
  for (size_t i = 1; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      fprintf(
          stderr,
          "Evaluator error: Type mismatch. arguments of function le must be "
          "integer\n");
      return NULL;
    }
    if (last->value > ((integer_t *)args[i])->value) {
      return make_nil();
    }
    last = (integer_t *)args[i];
  }
  return make_true();
}

value_t *eq(value_t **args) {
  value_t *last = args[0];
  for (size_t i = 1; args[i] != NULL; i++) {
    if (!compare_value(last, args[i])) {
      return make_nil();
    }
    last = args[i];
  }
  return make_true();
}

value_t *lor(value_t **args) {
  for (size_t i = 0; args[i] != NULL; i++) {
    if (!is_nil(args[i])) {
      return copy_value(args[i]);
    }
  }
  return make_nil();
}

value_t *land(value_t **args) {
  value_t *last;
  for (size_t i = 0; args[i] != NULL; i++) {
    if (is_nil(args[i])) {
      return make_nil();
    }
    last = args[i];
  }
  return copy_value(last);
}

value_t *lnot(value_t **args) {
  if (is_nil(args[0])) {
    return make_true();
  }
  return make_nil();
}

value_t *funcall(value_t **args) {
  switch (args[0]->type) {
    case CORE:
      value_t *result =
          search_cache(((core_function_t *)args[0])->name, args + 1);
      if (result == NULL) {
        result = ((core_function_t *)args[0])->body(args + 1);
        cache_result(((core_function_t *)args[0])->name, args + 1, result);
      }
      return result;
    case FUNC:
      scope_t *outer = get_scope();
      scope_t *scope = make_scope(outer);
      function_t *func = (function_t *)args[0];
      scope->closure = func->closure;
      result = search_cache(func->name, args + 1);
      if (result == NULL) {
        for (size_t i = 0; i < func->args_cnt; i++) {
          variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
          *var = (variable_t){
              .type = VAR,
              .name = strndup(func->args[i], strlen(func->args[i])),
              .data = args[i + 1]};
          args[i + 1] = NULL;
          add_symbol(scope, (value_t *)var);
        }
        result = evaluate(func->body, scope);
        cache_result(func->name, args + 1, result);
      }
      destroy_scope(scope);
      return result;
    default:
      fprintf(stderr,
              "Evaluator error: Type mismatch. The first argument of function "
              "funcall must be "
              "a function reference\n");
      return NULL;
  }
}

value_t *mapcar(value_t **args) {
  if (args[1]->type != LIST) {
    fprintf(stderr,
            "Evaluator error: Type mismatch. The second argument of function "
            "mapcar must be "
            "a list\n");
    return NULL;
  }
  list_t *list = (list_t *)args[1];
  list_t *list_start = list;
  list_t *result = (list_t *)calloc(1, sizeof(list_t));
  result->type = LIST;
  list_t *result_start = result;
  switch (args[0]->type) {
    case CORE:
      while (list != NULL) {
        args[1] = list->data;
        result->data =
            search_cache(((core_function_t *)args[0])->name, args + 1);
        if (result->data == NULL) {
          result->data = ((core_function_t *)args[0])->body(args + 1);
          cache_result(((core_function_t *)args[0])->name, args + 1,
                       result->data);
        }
        if (list->next != NULL) {
          result->next = (list_t *)calloc(1, sizeof(list_t));
          result = result->next;
          result->type = LIST;
        }
        list = list->next;
      }
      args[1] = (value_t *)list_start;
      break;
    case FUNC:
      function_t *func = (function_t *)args[0];
      scope_t *scope = get_scope();
      value_t **call_args = (value_t **)calloc(2, sizeof(value_t *));
      while (list != NULL) {
        scope_t *inner = make_scope(scope);
        inner->closure = func->closure;
        call_args[0] = list->data;
        result->data = search_cache(func->name, call_args);
        if (result->data == NULL) {
          variable_t *var = (variable_t *)calloc(1, sizeof(variable_t));
          *var = (variable_t){
              .type = VAR,
              .name = strndup(func->args[0], strlen(func->args[0])),
              .data = copy_value(list->data)};
          add_symbol(inner, (value_t *)var);
          result->data = evaluate(func->body, inner);
          cache_result(func->name, call_args, result->data);
        }
        destroy_scope(inner);
        if (list->next != NULL) {
          result->next = (list_t *)calloc(1, sizeof(list_t));
          result = result->next;
          result->type = LIST;
        }
        list = list->next;
      }
      args[1] = (value_t *)list_start;
      free(call_args);
      break;
    default:
      fprintf(stderr,
              "Evaluator error: Type mismatch. The first argument of function "
              "mapcar must be "
              "a function reference\n");
      destroy_value((value_t *)result);
      return NULL;
  }
  return (value_t *)result_start;
}
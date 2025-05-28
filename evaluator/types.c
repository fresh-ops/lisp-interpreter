#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "scope.h"

static void destroy_integer(integer_t *value) { free(value); }

static void destroy_string(string_t *value) {
  free(value->value);
  free(value);
}

static void destroy_variable(variable_t *value) {
  destroy_value(value->data);
  free(value->name);
  free(value);
}

static void destroy_core_function(core_function_t *value) {
  free(value->name);
  free(value);
}

static void destroy_function(function_t *value) {
  for (size_t i = 0; i < value->args_cnt; i++) {
    free(value->args[i]);
  }
  free(value->args);
  free(value->name);
  destroy_scope_rec((scope_t *)value->closure);
  destroy_tree(value->body, 1, 1);
  free(value);
}

static void destroy_list(list_t *value) {
  if (value == NULL) {
    return;
  }
  destroy_value(value->data);
  destroy_list(value->next);
  free(value);
}

void destroy_value(value_t *value) {
  if (value == NULL) {
    return;
  }
  switch (value->type) {
    case INT:
      destroy_integer((integer_t *)value);
      break;
    case STR:
      destroy_string((string_t *)value);
      break;
    case VAR:
      destroy_variable((variable_t *)value);
      break;
    case CORE:
      destroy_core_function((core_function_t *)value);
      break;
    case FUNC:
      destroy_function((function_t *)value);
      break;
    case LIST:
      destroy_list((list_t *)value);
      break;
    case TRUE:
      free(value);
      break;
    default:
      break;
  }
}

static value_t *copy_integer(integer_t *value) {
  integer_t *copy = (integer_t *)calloc(1, sizeof(integer_t));
  memcpy(copy, value, sizeof(integer_t));
  return (value_t *)copy;
}

static value_t *copy_string(string_t *value) {
  string_t *copy = (string_t *)calloc(1, sizeof(string_t));
  memcpy(copy, value, sizeof(string_t));
  copy->value = strndup(value->value, value->length);
  return (value_t *)copy;
}

static value_t *copy_variable(variable_t *value) {
  variable_t *copy = (variable_t *)calloc(1, sizeof(variable_t));
  copy->type = VAR;
  copy->name = strdup(value->name);
  copy->data = copy_value(value->data);
  return (value_t *)copy;
}

static value_t *copy_core_function(core_function_t *value) {
  core_function_t *copy = (core_function_t *)calloc(1, sizeof(core_function_t));
  memcpy(copy, value, sizeof(core_function_t));
  copy->name = strdup(value->name);
  return (value_t *)copy;
}

static value_t *copy_function(function_t *value) {
  function_t *copy = (function_t *)calloc(1, sizeof(function_t));
  copy->type = FUNC;
  copy->name = (value->name != NULL) ? strdup(value->name) : NULL;
  copy->args_cnt = value->args_cnt;
  copy->args = (char **)calloc(copy->args_cnt, sizeof(char *));
  for (size_t i = 0; i < copy->args_cnt; i++) {
    copy->args[i] = strdup(value->args[i]);
  }
  copy->body = copy_tree(value->body);
  return (value_t *)copy;
}

static value_t *copy_list(list_t *value) {
  if (value == NULL) {
    return NULL;
  }
  list_t *copy = (list_t *)calloc(1, sizeof(list_t));
  copy->type = LIST;
  copy->data = copy_value(value->data);
  copy->next = copy_list(value->next);
  return (value_t *)copy;
}

value_t *copy_value(value_t *value) {
  if (value == NULL) {
    return NULL;
  }
  switch (value->type) {
    case INT:
      return copy_integer((integer_t *)value);
    case STR:
      return copy_string((string_t *)value);
    case VAR:
      return copy_variable((variable_t *)value);
    case CORE:
      return copy_core_function((core_function_t *)value);
    case FUNC:
      return copy_function((function_t *)value);
    case LIST:
      return copy_list((list_t *)value);
    case TRUE:
      return make_true();
    default:
      return NULL;
  }
}

int is_nil(value_t *value) {
  if (value == NULL) {
    return 1;
  }
  if (value->type == LIST) {
    list_t *list = value;
    if (list->data == NULL && list->next == NULL) {
      return 1;
    }
  }
  return 0;
}

value_t *make_nil() {
  list_t *value = (list_t *)calloc(1, sizeof(list_t));
  value->type = LIST;
  return (value_t *)value;
}

value_t *make_true() {
  value_t *result = (value_t *)calloc(1, sizeof(value_t));
  result->type = TRUE;
  return result;
}
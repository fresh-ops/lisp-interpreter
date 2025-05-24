#include "types.h"

#include <stdlib.h>

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
  destroy_tree(value->body, 1, 1);
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
    default:
      break;
  }
}
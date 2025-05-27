#include "core.h"

value_t *sum(value_t **args) {
  integer_t *result = (integer_t *)calloc(1, sizeof(integer_t));
  *result = (integer_t){.type = INT, .value = 0};

  for (size_t i = 0; args[i] != NULL; i++) {
    if (args[i]->type != INT) {
      free(result);
      return NULL;
    }
    result->value += ((integer_t *)args[i])->value;
  }

  return (value_t *)result;
}

value_t *sub(value_t **args) {
  if (args[0] == NULL || args[0]->type != INT) {
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

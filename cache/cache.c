#include "cache.h"

#include <stdlib.h>
#include <string.h>

typedef struct call {
  value_t **args;
  value_t *result;
  size_t cnt;
  size_t cap;
} call_t;

typedef struct cache {
  char *name;
  call_t *calls;
  size_t cap;
  size_t cnt;
} cache_t;

static cache_t *bank = NULL;
static size_t cap = 0;
static size_t cnt = 0;

void init_cache() {
  if (bank == NULL) {
    bank = (cache_t *)calloc(4, sizeof(cache_t));
    cap = 4;
    cnt = 0;
  }
}

void destroy_cache() {
  if (bank == NULL) {
    return;
  }

  for (size_t i = 0; i < cnt; i++) {
    free(bank[i].name);
    for (size_t j = 0; j < bank[i].cnt; j++) {
      for (size_t k = 0; k < bank[i].calls[j].cnt; k++) {
        destroy_value(bank[i].calls[j].args[k]);
      }
      free(bank[i].calls[j].args);
      destroy_value(bank[i].calls[j].result);
    }
    free(bank[i].calls);
  }
  free(bank);
  bank = NULL;
}

static int compare_args(value_t **args1, value_t **args2) {
  size_t i;
  for (i = 0; args1[i] != NULL && args2[i] != NULL; i++) {
    if (!compare_value(args1[i], args2[i])) {
      return 0;
    }
  }
  return args1[i] == args2[i];
}

void cache_result(char *name, value_t **args, value_t *result) {
  init_cache();
  if (name == NULL) return;

  size_t ind = 0;
  for (ind = 0; ind < cnt; ind++) {
    if (strcmp(name, bank[ind].name) == 0) {
      break;
    }
  }

  if (ind >= cnt) {
    if (cnt >= cap) {
      cap *= 2;
      bank = (cache_t *)realloc(bank, cap * sizeof(cache_t));
    }
    bank[cnt].name = strdup(name);
    bank[cnt].cap = 4;
    bank[cnt].cnt = 0;
    bank[cnt].calls = (call_t *)calloc(4, sizeof(call_t));
    cnt++;
  }

  cache_t *func_cache = &bank[ind];

  for (size_t i = 0; i < func_cache->cnt; i++) {
    if (compare_args(args, func_cache->calls[i].args)) {
      destroy_value(func_cache->calls[i].result);
      func_cache->calls[i].result = copy_value(result);
      return;
    }
  }

  if (func_cache->cnt >= func_cache->cap) {
    func_cache->cap *= 2;
    func_cache->calls =
        (call_t *)realloc(func_cache->calls, func_cache->cap * sizeof(call_t));
  }

  call_t *new_call = &func_cache->calls[func_cache->cnt++];
  new_call->cap = 4;
  new_call->cnt = 0;
  new_call->args = (value_t **)calloc(4, sizeof(value_t *));

  for (size_t i = 0; args[i] != NULL; i++) {
    if (new_call->cnt >= new_call->cap) {
      new_call->cap *= 2;
      new_call->args = (value_t **)realloc(new_call->args,
                                           new_call->cap * sizeof(value_t *));
    }
    new_call->args[new_call->cnt++] = copy_value(args[i]);
  }
  new_call->result = copy_value(result);
}

value_t *search_cache(char *name, value_t **args) {
  if (bank == NULL) return NULL;
  if (name == NULL) return NULL;

  for (size_t i = 0; i < cnt; i++) {
    if (strcmp(name, bank[i].name) == 0) {
      cache_t *func_cache = &bank[i];

      for (size_t j = 0; j < func_cache->cnt; j++) {
        if (compare_args(args, func_cache->calls[j].args)) {
          return copy_value(func_cache->calls[j].result);
        }
      }
      break;
    }
  }
  return NULL;
}
#include <stdio.h>
#include <string.h>

#include "cache.h"
#include "core.h"
#include "evaluator.h"
#include "parser.h"

int stop = 0;

void show_value(value_t *value) {
  if (is_nil(value)) {
    printf("NIL");
    return;
  }
  switch (value->type) {
    case INT:
      printf("%lld", ((integer_t *)value)->value);
      break;
    case STR:
      printf("\"%s\"", ((string_t *)value)->value);
      break;
    case VAR:
      printf("%s", ((variable_t *)value)->name);
      // show_value(((variable_t *)value)->data);
      break;
    case FUNC:
      printf("%s", ((function_t *)value)->name);
      break;
    case LIST:
      printf("(");
      list_t *list = (list_t *)value;
      while (list != NULL) {
        show_value(list->data);
        list = list->next;
        if (list != NULL) {
          printf(" ");
        }
      }
      printf(")");
      break;
    case TRUE:
      printf("T");
      break;
    default:
      printf("Undefined");
      break;
  }
}

value_t *quit(value_t **args) {
  printf("Bye.\n");
  stop = 1;
  return NULL;
}

void add_core(scope_t *scope) {
  core_function_t *func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func =
      (core_function_t){.name = strndup("sum\0", 4), .body = sum, .type = CORE};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strndup("exit\0", 5), .body = quit, .type = CORE};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("sub\0"), .body = sub, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("gt\0"), .body = gt, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("lt\0"), .body = lt, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("ge\0"), .body = ge, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("le\0"), .body = le, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("eq\0"), .body = eq, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("or\0"), .body = lor, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("and\0"), .body = land, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("not\0"), .body = lnot, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){.name = strdup("funcall\0"),
                            .body = funcall,
                            .type = CORE,
                            .args_cnt = 2};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strdup("mapcar\0"), .body = mapcar, .type = CORE, .args_cnt = 2};
  add_symbol(scope, (value_t *)func);
}

int main() {
  char input[100];

  scope_t *scope = make_scope(NULL);
  add_core(scope);
  init_cache();
  while (1) {
    printf("> ");
    fgets(input, 99, stdin);
    for (size_t j = 0; input[j] != '\0'; j++) {
      if (input[j] == '\n') {
        input[j] = '\0';
      }
    }
    as_tree_t *tree = parse(input);
    if (tree == NULL) {
      continue;
    }
    value_t *result = evaluate(tree, scope);
    if (stop) {
      destroy_value(result);
      destroy_tree(tree, 1, 0);
      break;
    }
    show_value(result);
    printf("\n");
    destroy_value(result);
    destroy_tree(tree, 1, 0);
  }
  destroy_scope(scope);
  destroy_cache();
  return 0;
}
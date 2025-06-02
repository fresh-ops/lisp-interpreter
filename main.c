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
      .name = strndup("del\0", 5), .body = del, .type = CORE, .args_cnt = 2};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strndup("list\0", 5), .body = list, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strndup("head\0", 5), .body = head, .type = CORE, .args_cnt = 1};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strndup("cat\0", 5), .body = cat, .type = CORE, .args_cnt = 2};
  add_symbol(scope, (value_t *)func);

  func = (core_function_t *)calloc(1, sizeof(core_function_t));
  *func = (core_function_t){
      .name = strndup("tail\0", 5), .body = tail, .type = CORE, .args_cnt = 1};
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

void print_help() {
  printf("LISP INTERPRETER HELP MANUAL\n");
  printf("===========================\n\n");

  printf("SYNOPSIS\n");
  printf("    (function arg1 arg2 ...)\n\n");

  printf("CORE FUNCTIONS\n");
  printf("    Arithmetic:\n");
  printf("    (sum a b)                 - Returns a + b\n");
  printf("    (sub a b)                 - Returns a - b\n\n");

  printf("    Comparisons (work with 2+ arguments):\n");
  printf("    (gt a b c ...)            - Returns true if a > b > c > ...\n");
  printf("    (lt a b c ...)            - Returns true if a < b < c < ...\n");
  printf(
      "    (ge a b c ...)            - Returns true if a >= b >= c >= ...\n");
  printf(
      "    (le a b c ...)            - Returns true if a <= b <= c <= ...\n");
  printf(
      "    (eq a b c ...)            - Returns true if all arguments are "
      "equal\n\n");

  printf("    Functional:\n");
  printf("    (funcall fn ...)          - Calls function fn with arguments\n");
  printf(
      "    (mapcar fn list)          - Applies fn to each element of list\n");
  printf("    (exit)                    - Quits the interpreter\n\n");

  printf("  List operations:\n");
  printf("  (head xs)                   - Returns the head of list\n");
  printf("  (tail xs)                   - Returns the tail of list\n");
  printf("  (list x)                    - Wraps x in a list\n");
  printf("  (cat xs ys)                 - Concatenates two lists\n");
  printf("  (del xs x)                  - Removes first occurence of x\n\n");

  printf("SYNTAX\n");
  printf("    Lists:                    '(1 2 3)\n");
  printf("    Function ref:             #'function-name\n");
  printf("    Lambda:                   (lambda (args) body)\n\n");

  printf("DEFINITIONS\n");
  printf("    (defvar name value)      - Defines a variable\n");
  printf("    (defun name (args) body) - Defines a function\n\n");

  printf("Commands\n");
  printf("    :h                       - Prints this manual\n\n");
}

int main(int argc, char **argv) {
  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0) {
      print_help();
      return 0;
    }
    printf("Undefined flag %s. Try to use flag --help to see manual\n",
           argv[1]);
    return 0;
  }
  if (argc > 2) {
    printf("Too many arguments passed. Try to use flag --help to see manual\n");
    return 0;
  }
  char input[100];

  scope_t *scope = make_scope(NULL);
  add_core(scope);
  init_cache();
  while (1) {
    printf("> ");
    if (!fgets(input, 99, stdin)) {
      printf("\n");
      break;
    }
    for (size_t j = 0; input[j] != '\0'; j++) {
      if (input[j] == '\n') {
        input[j] = '\0';
      }
    }
    if (strcmp(input, ":h") == 0) {
      print_help();
      continue;
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
  printf("Bye.\n");
  return 0;
}
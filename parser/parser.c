#include "parser.h"

#include <stdio.h>
#include <string.h>

#include "lexer.h"

static as_tree_t *parse_expression(const char *input);

static size_t skip_whitespaces(const char *input) {
  const char *start = input;
  while (*input == ' ') {
    input++;
  }
  return input - start;
}

static as_tree_t *parse_symbol(const char *input, int quoted) {
  const char *start = input;
  while (*input == '\'') {
    input++;
  }
  if (*input == '(') {
    input++;
    as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
    *tree = (as_tree_t){.type = QUOTED,
                        .cap = 4,
                        .cnt = 0,
                        .children = (as_tree_t *)calloc(4, sizeof(as_tree_t))};
    while (*input != ')' && *input != '\0') {
      as_tree_t *child = parse_symbol(input, quoted);
      if (child == NULL) {
        destroy_tree(tree, 1, 0);
        return NULL;
      }
      if (tree->cnt >= tree->cap) {
        tree->cap *= 2;
        tree->children =
            (as_tree_t *)realloc(tree->children, tree->cap * sizeof(as_tree_t));
      }

      memcpy(&tree->children[tree->cnt++], child, sizeof(as_tree_t));
      input += child->length;
      input += skip_whitespaces(input);
      free(child);
    }
    if (*input != ')') {
      fprintf(stderr, "Parser error: Unexpected end of line. Expected \')\'\n");
      destroy_tree(tree, 1, 0);
      return NULL;
    }
    tree->length = input - start + 1;
    return tree;
  }
  token_t *token = parse_atom(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));

  if (tree == NULL) {
    free(token);
    return NULL;
  }

  tree->token = token;
  tree->length = token->start + token->length - start;
  if (token->type == TOKEN_STR) {
    tree->length += 2;  // длинна строки не учитывает ковычки
  }
  tree->type = (token->type == TOKEN_ID) ? NAME : VALUE;
  if (quoted) {
    tree->type = QUOTED;
  }

  return tree;
}

as_tree_t *parse(const char *input) {
  static size_t depth = 0;
  depth++;
  as_tree_t *tree = NULL;
  if (*input == '(') {
    tree = parse_expression(input);
  } else if (*input == '#') {
    if (*(input + 1) == '\'') {
      token_t *token = parse_identifier(input + 2);
      if (token == NULL) {
        fprintf(stderr,
                "Parser error: Expected an identifier\nError occured while "
                "parsing \"%s\"\n",
                input);
        return NULL;
      }

      tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));

      if (tree == NULL) {
        free(token);
        return NULL;
      }

      tree->token = token;
      tree->type = REFERENCE;
      tree->length = token->length + 2;
      return tree;
    }
    fprintf(stderr,
            "Parser error: Unexpected character \'%c\'. Expected \'\'\'\nError "
            "occured while parsing \"%s\"\n",
            *(input + 1), input);
    return NULL;
  } else {
    int quoted = 0;
    if (*input == '\'') {
      quoted = 1;
    }
    tree = parse_symbol(input, quoted);
  }
  depth--;
  if (depth == 0) {
    size_t parsed_len = (tree) ? tree->length : 0;
    const char *unparsed = input + parsed_len;
    unparsed += skip_whitespaces(unparsed);
    if (*unparsed != '\0') {
      fprintf(stderr,
              "Parser error: Unxpected characters '%s'. Expected end of line\n",
              unparsed);
      destroy_tree(tree, 1, 0);
      return NULL;
    }
  }
  return tree;
}

static as_tree_t *parse_expression(const char *input) {
  const char *start = input++;
  if (*input == ')') {
    as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
    tree->length = 2;
    tree->type = QUOTED;
    return tree;
  }
  token_t *token = parse_identifier(input);
  if (token == NULL) {
    return NULL;
  }

  as_tree_t *tree = (as_tree_t *)calloc(1, sizeof(as_tree_t));
  if (tree == NULL) {
    free(token);
    return NULL;
  }
  *tree = (as_tree_t){.type = EXPRESSION,
                      .token = token,
                      .cap = 2,
                      .cnt = 0,
                      .children = (as_tree_t *)calloc(2, sizeof(as_tree_t))};

  input += token->length;
  input += skip_whitespaces(input);

  while (*input != ')' && *input != '\0') {
    if (tree->cnt >= tree->cap) {
      tree->cap *= 2;
      tree->children =
          (as_tree_t *)realloc(tree->children, tree->cap * sizeof(as_tree_t));
    }

    as_tree_t *child = parse(input);
    if (child == NULL) {
      destroy_tree(tree, 1, 0);
      return NULL;
    }

    memcpy(tree->children + tree->cnt++, child, sizeof(as_tree_t));
    input += child->length;
    input += skip_whitespaces(input);
    free(child);
  }
  if (*input != ')') {
    fprintf(stderr, "Parser error: Unexpected end of line. Expected \')\'\n");
    destroy_tree(tree, 1, 0);
    return NULL;
  }
  tree->length = input - start + 1;
  return tree;
}
#include "parser.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char *extract_token(const token_t *token) {
  if (token == NULL || token->start == NULL) {
    return NULL;
  }

  char *result = (char *)calloc(token->length + 1, sizeof(char));
  if (result == NULL) {
    return NULL;
  }

  size_t copy_len = token->length;
  if (copy_len > 0) {
    memcpy(result, token->start, copy_len);
  }
  result[copy_len] = '\0';

  return result;
}

token_t *make_token(enum token_type type, const char *start, size_t length) {
  if (start == NULL) {
    return NULL;
  }

  token_t *token = (token_t *)malloc(sizeof(token_t));
  if (token == NULL) {
    return NULL;
  }

  *token = (token_t){.type = type, .start = start, .length = length};

  return token;
}

token_t *parse_integer(const char *input) {
  if (input == NULL || *input == '\0') {
    return NULL;
  }

  const char *start = input;
  int has_sign = 0;
  if (*input == '+' || *input == '-') {
    has_sign = 1;
    input++;
  }

  if (*input == '0' && isdigit((unsigned char)*(input + 1))) {
    return NULL;
  }

  while (isdigit((unsigned char)*input)) {
    input++;
  }
  if (input - start == has_sign) {
    return NULL;
  }
  return make_token(TOKEN_INT, start, input - start);
}

token_t *parse_string(const char *input) {
  if (input == NULL || *input != '"') {
    return NULL;
  }

  const char *start = input++;
  int escaped = 0;

  while (*input != '\0') {
    if (*input == '"' && !escaped) {
      return make_token(TOKEN_STR, start, input - start + 1);
    } else if (*input == '\\' && !escaped) {
      escaped = 1;
    } else {
      escaped = 0;
    }
    input++;
  }
  return NULL;
}

token_t *parse_identifier(const char *input) {
  if (input == NULL || !isalpha((unsigned char)*input)) {
    return NULL;
  }

  const char *start = input++;
  while (*input != '\0' && isalnum((unsigned char)*input)) {
    input++;
  }
  return make_token(TOKEN_ID, start, input - start);
}
#pragma once
#include <stdlib.h>

enum token_type { TOKEN_INT, TOKEN_STR, TOKEN_EXPR };

typedef struct token {
  enum token_type type;
  char *start;
  size_t length;
} token_t;

/**
 * @brief Extracts the token text into a new null-terminated string
 *
 * @param token The token to extract
 * @return char* Newly allocated string containing token text, or NULL on
 * failure
 */
char *extract_token(const token_t *token);

/**
 * @brief Creates a new token
 *
 * @param type Token type
 * @param start Start of token text (must not be NULL)
 * @param length Length of token
 * @return token_t* New token, or NULL on failure
 */
token_t *make_token(enum token_type type, const char *start, size_t length);

/**
 * @brief Parses a number from input
 *
 * @param input Input string (must not be NULL)
 * @return token_t* New token if number found, NULL otherwise
 */
token_t *parse_integer(const char *input);
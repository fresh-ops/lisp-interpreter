#pragma once
#include "../models/astree.h"

/**
 * @brief Parses input into an abstract syntax tree
 *
 * @param input Input string to parse (must not be NULL)
 * @return as_tree_t* Root node of the AST on success, NULL on failure
 */
as_tree_t *parse(const char *input);
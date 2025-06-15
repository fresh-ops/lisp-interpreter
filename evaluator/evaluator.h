#pragma once

#include "astree.h"
#include "scope.h"
#include "types.h"

/**
 * @brief Evaluates the tree
 *
 * @param tree The tree to evaluate (must not be NULL)
 * @param scope The scope with defined variables and functions
 * @return value_t* Result of tree evaluation
 */
value_t *evaluate(const as_tree_t *tree, scope_t *scope);
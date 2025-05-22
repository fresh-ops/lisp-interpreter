#pragma once

#include "../models/astree.h"
#include "types.h"
#include "scope.h"

/**
 * @brief Evaluates the tree
 * 
 * @param tree The tree to evaluate (must not be NULL)
 * @param scope The scope with defined variables and functions
 * @return value_t* Result of tree evaluation  
 */
value_t *evaluate(const as_tree_t *tree, scope_t *scope);
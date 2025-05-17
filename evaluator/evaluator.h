#include "../models/astree.h"
#include "types.h"

/**
 * @brief Evaluates the tree
 * 
 * @param tree The tree to evaluate (must not be NULL)
 * @return value_t* Result of tree evaluation  
 */
value_t *evaluate(const as_tree_t *tree);
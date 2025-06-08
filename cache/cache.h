#include "types.h"

void init_cache();

void destroy_cache();

void cache_result(char *name, value_t **args, value_t *result);

value_t *search_cache(char *name, value_t **args);
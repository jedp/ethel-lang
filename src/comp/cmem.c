#include <stdlib.h>

#include "cmem.h"

void *comp_alloc(size_t size) {
    return malloc(size);
}

void *comp_realloc(void *ptr, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, new_size);
}

void comp_free(void *ptr) {
    free(ptr);
    ptr = NULL;
}

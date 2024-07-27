#include <stdlib.h>

#include "mem.h"

void *mem_alloc(uint32_t size) {
    return malloc(size);
}

void *mem_realloc(void *ptr, uint32_t old_size, uint32_t new_size) {
    (void) old_size;

    if (new_size == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, new_size);
}

void mem_free(void *ptr) {
    free(ptr);
    ptr = NULL;
}

#pragma once

#include <stdint.h>

#define ARR_GROW_MAX_SIZE(new_max) ((new_max) < 8 ? 8 : (new_max) * 2)

#define ARR_GROW(type, arr, size) \
    (type *) comp_realloc(arr, sizeof(type) * size)

void *comp_alloc(size_t size);

void *comp_realloc(void *ptr, size_t new_size);

void comp_free(void *ptr);
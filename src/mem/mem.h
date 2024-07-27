#pragma once

#include <stdint.h>

#define ARR_GROW_MAX_SIZE(new_max) \
    ((new_max) < 8 ? 8 : (new_max) * 2)

#define ARR_GROW(type, arr, size) \
    (type *) mem_realloc(arr, 0, sizeof(type) * size)

void *mem_alloc(uint32_t size);

void *mem_realloc(void *ptr, uint32_t old_size, uint32_t new_size);

void mem_free(void *ptr);

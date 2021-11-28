#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"

void* mem_alloc(dim_t size) {
  return malloc(size);
}

void* mem_realloc(void *b, dim_t size) {
  return realloc(b, size);
}

void mem_free(void *b) {
  free(b);
}


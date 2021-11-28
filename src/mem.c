#include <stdlib.h>
#include "../inc/mem.h"

#ifdef USE_ETHEL_MEM
#include "../inc/heap.h"
#else
#include <stdio.h>
#endif

void* mem_alloc(dim_t size) {
#ifdef USE_ETHEL_MEM
  return ealloc(size);
#else
  return malloc(size);
#endif
}

void* mem_realloc(void *b, dim_t size) {
#ifdef USE_ETHEL_MEM
  return erealloc(b, size);
#else
  return realloc(b, size);
#endif
}

void mem_free(void *b) {
#ifdef USE_ETHEL_MEM
  return efree(b);
#else
  free(b);
#endif
}

void mem_init(void) {
#ifdef USE_ETHEL_MEM
  heap_init();
#endif
}


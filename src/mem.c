#include "../inc/mem.h"
#include <stdlib.h>

void* mem_alloc(dim_t size) {
  return malloc(size);
}

void mem_set(void *b, int val, unsigned int len) {
  int *p = (int*) b;
  for (int i = 0; i < len; i++) {
    *p++ = val; 
  }
}


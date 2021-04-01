#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"

void* mem_alloc(dim_t size) {
  return malloc(size);
}

void mem_set(void *b, int val, dim_t len) {
  unsigned char *p = b;
  while (len-- > 0) *p++ = val;
}


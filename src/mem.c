#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"

void* mem_alloc(dim_t size) {
  return malloc(size);
}

void* mem_realloc(void *b, dim_t size) {
  return realloc(b, size);
}

void* mem_cp(void *dst, void *src, dim_t size) {
  unsigned char *p1 = (unsigned char*) dst;
  const unsigned char *p2 = (const unsigned char*) src;

  while (size-- > 0) *p1++ = *p2++;
  return dst;
}

void mem_free(void *b) {
  free(b);
}

void mem_set(void *b, int val, dim_t len) {
  unsigned char *p = b;
  while (len-- > 0) *p++ = val;
}


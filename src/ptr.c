#include "../inc/ptr.h"

void mem_set(void *b, int val, dim_t len) {
  unsigned char *p = b;
  while (len-- > 0) *p++ = val;
}

void* mem_cp(void *dst, void *src, dim_t size) {
  unsigned char *p1 = (unsigned char*) dst;
  const unsigned char *p2 = (const unsigned char*) src;

  while (size-- > 0) *p1++ = *p2++;
  return dst;
}


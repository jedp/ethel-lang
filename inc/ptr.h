#ifndef __PTR_H
#define __PTR_H

#include "def.h"

/* Set the first len bytes in b to val. */
void mem_set(void *b, int val, dim_t len);

/* Copy n bytes from src to dst. Return pointer to dst. */
void* mem_cp(void* dst, void* src, dim_t size);

#endif


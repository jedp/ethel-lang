#ifndef __MEM_H
#define __MEM_H

#include "def.h"

/*
 * Allocate size bytes of memory, returning a pointer to the allocated block.
 * If the pointer is null, no memory was available.
 */
void* mem_alloc(dim_t size);

/*
 * Re-allocate memory object b to occupy size bytes. If insufficient memory
 * was available, return null pointer.
 */
void* mem_realloc(void *b, dim_t size);

/* Copy n bytes from src to dst. Return pointer to dst. */
void* mem_cp(void* dst, void* src, dim_t size);

/* Free allocated memory. */
void mem_free(void *b);

/* Set the first len bytest in b to val. */
void mem_set(void *b, int val, dim_t len);

#endif


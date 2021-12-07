#ifndef __MEM_H
#define __MEM_H

#include "def.h"

/*
 * Allocate size bytes of memory, returning a pointer to the allocated block.
 * If the pointer is null, no memory was available.
 */
void* mem_alloc(size_t size);

/*
 * Re-allocate memory object b to occupy size bytes. If insufficient memory
 * was available, return null pointer.
 */
void* mem_realloc(void *b, size_t size);

/* Free allocated memory. */
void mem_free(void *b);

/* Initialize memory management. Initialize all words with initval. */
void mem_init(unsigned char initval);

/* Tag the object so it can be traced in GC. */
void mark_traceable(void* obj, type_t type, flags_t flags);

#endif


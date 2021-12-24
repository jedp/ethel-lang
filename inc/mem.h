#ifndef __MEM_H
#define __MEM_H

#include "def.h"

/*
 * Allocate an object of size type_t with the given flags.
 *
 * Makes the allocated object traceable by GC.
 */
gc_header_t* alloc_type(type_t type, flags_t flags);

/*
 * Allocate size bytes of memory, returning a pointer to the allocated block.
 * If the pointer is null, no memory was available.
 *
 * Does not make the object traceable by GC.
 */
void* mem_alloc(size_t size);

/*
 * Re-allocate memory object b to occupy size bytes. If insufficient memory
 * was available, return null pointer.
 *
 * Does not make the object traceable by GC.
 */
void* mem_realloc(void *b, size_t size);

/* Free allocated memory. */
void mem_free(void *b);

/* Initialize memory management. Initialize all words with initval. */
void mem_init(unsigned char initval);

void assert_valid_typed_node(gc_header_t* node);

#endif


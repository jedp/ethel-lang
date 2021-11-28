#ifndef __HEAP_H
#define __HEAP_H

#include <stdint.h>
#include "../inc/def.h"

// Node flags.
#define F_FREE (1 << 0)  // Node is on the free list.

/*
 * The heap is a simple doubly-linked list of nodes.
 *
 * A node is flagged F_FREE if it is available for allocation, or not if it is
 * in use.
 *
 * Size designates the number of available bytes in this node.
 *
 * There is no pointer to the data buffer here. Instead, ealloc() and efree()
 * figure it out via pointer arithmetic.
 *
 * Thus when a program calls ealloc(), the pointer returned will not be a
 * pointer to a heap_node_t, but to the first word after it.
 */
typedef struct HeapNode {
  struct HeapNode *prev;
  struct HeapNode *next;
  uint32_t size;
  uint32_t flags;
} heap_node_t;

// Heap size must be a multiple of heap_node_t size.
// Remove a smidge if necessary.
#define HEAP_BYTES (HEAP_SIZE_BYTES - HEAP_SIZE_BYTES % sizeof(heap_node_t))

typedef struct {
  uint32_t total_nodes;
  uint32_t free_nodes;
  uint32_t bytes_used;
  uint32_t bytes_free;
} heap_info_t;

/*
 * Initialize the heap. Do this once.
 *
 * Rebuilds the heap data structure, which means any lingering pointers to data
 * on the heap should not be used again. Intended to be done once on first
 * execution. Exposed for testing.
 */
void heap_init();

/*
 * Try to allocate the given number of bytes. Allocations are always a multiple
 * of the size of the heap_node_t struct, so the actual number of bytes
 * allocated may be larger than what was requested. Additionally, each new
 * block requires a block of size heap_node_t for bookkeeping.
 *
 * Return a pointer to the data buffer of the block allocated.
 *
 * Return NULL if memory could not be allocated.
 */
void *ealloc(uint32_t bytes);

/*
 * Free the nodes associated with the data_ptr. The given data_ptr must be an
 * address returned by ealloc(), or things may start to get wild.
 *
 * The freed block is made available for allocation.
 *
 * If the freed block is adjacent to any other free blocks, they will all
 * coalesce into a single block on the free list.
 */
void efree(void *data_ptr);

/*
 * Traverse the heap and examine it. Useful for assertions in tests.
 */
heap_info_t *get_heap_info();

#endif


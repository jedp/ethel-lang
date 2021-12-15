#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/gc.h"
#include "../inc/heap.h"
#include "../inc/env.h"

/*
 * Baker's Mark and Sweep algorithm, as described in Aho et al., Compilers,
 * Chapter 7.
 *
 * INPUT:
 * - Root set of objects
 * - The heap
 * - The list of all free blocks on the heap ("Free")
 * - A list of allocated objects ("Unreached")
 *
 * OUTPUT: Modified lists "Free" and "Unreached", which holds allocated
 * objects.
 *
 * METHOD:
 *
 * Maintain four lists: "Free", "Unreached", "Unscanned", and "Scanned".
 * Objects on the heap are marked with bits signifying membership in these
 * lists.  Initially, "Free" is the free list maintained by the memory manager,
 * and all allocated objects on the heap are in the "Unreached" list.
 *
 * - Initialize Scanned to be the empty list.
 * - Initialize Unscanned to have only the objects reached by the root set.
 * - While there are unscanned nodes
 *   - Move object o rom Unscanned to Scanned
 *   - For each object o' referenced by o
 *     - If o' is Unreached
 *       Move o' from Unreached to Unscanned
 * - Free = Free + Unreached
 * - Unreached = Scanned
 */

#define F_GC_UNSET ~( F_GC_UNREACHED | F_GC_UNSCANNED )

static void coalesce_free_nodes(void) {
  heap_node_t *heap_node = heap_head();

  // Starting at the beginning of the heap, find the next free node and
  // absorb all subsequent free nodes into it. Repeat until there are no
  // more nodes.
  while (heap_node != NULL) {
    if (heap_node->flags & F_GC_FREE) {
      while (heap_node->next != NULL && (heap_node->next->flags & F_GC_FREE)) {
        // Coalesce node->next into node.
        heap_node->next = heap_node->next->next;
      }
    }
    heap_node = heap_node->next;
  }
}

static void move_unreached_to_free(void) {
  heap_node_t *heap_node = heap_head();

  while (heap_node != NULL) {
    if (heap_node->flags & F_GC_UNREACHED) {
      // Mark as free, but don't use efree; we will coalesce nodes later.
      assert(!(heap_node->flags & F_GC_UNSCANNED));
      heap_node->flags = F_GC_FREE;
    }
    heap_node = heap_node->next;
  }
}

static void move_unreached_to_unscanned(heap_node_t *heap_node) {
  // Prevent cycles.
  if (!(heap_node->flags & F_GC_UNREACHED)) return;

  heap_node->flags &= ~(F_GC_UNREACHED);
  heap_node->flags |= F_GC_UNSCANNED;

  // Inspect the data contained in the heap node:
  // Find any child data nodes and and scan their respective heap nodes.
  void* data = DATA_FOR_NODE(heap_node);
  gc_header_t* data_hdr = (gc_header_t*) data;
  if (data_hdr->children == 0) return;

  for (int i = 0; i < data_hdr->children; ++i) {
    size_t offset = sizeof(gc_header_t) + (i * sizeof(void*));
    void **child = (void*) (data_hdr) + offset;
    // Pointers can be null. elem->next, etc.
    if (*child != NULL) {
      move_unreached_to_unscanned(NODE_FOR_DATA(*child));
    }
  }
}

static int scan_unscanned_objects() {
  heap_node_t* heap_node = heap_head();

  int unscanned = 0;

  while (heap_node != NULL) {
    if (heap_node->flags & F_GC_UNSCANNED) {
      heap_node->flags &= ~F_GC_UNSCANNED;

      unscanned += 1;

      move_unreached_to_unscanned(heap_node);
    }

    heap_node = heap_node->next;
  }

  return unscanned;
}

static void move_root_unreached_to_unscanned(env_t* env) {
  assert(env->top >= 0);

  // Move objects referenced by the root set from Unreached to Unscanned.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *env_node = env->symbols[i];
    while (env_node != NULL) {
      heap_node_t* heap_node = NODE_FOR_DATA(env_node);
      move_unreached_to_unscanned(heap_node);
      env_node = env_node->next;
    }
  }
}

static void initialize_unreached(void) {
  heap_node_t* heap_node = heap_head();
  while (heap_node != NULL) {

    // Scanned = Unscanned = 0.
    heap_node->flags &= F_GC_UNSET;

    // All allocated objects are Unreached.
    if (!(heap_node->flags & F_GC_FREE)) {
      heap_node->flags |= F_GC_UNREACHED;
    }

    heap_node = heap_node->next;
  }
}

void gc(env_t *env) {
  //dump_heap();
  size_t used_before = get_heap_info()->bytes_used;

  initialize_unreached();
  move_root_unreached_to_unscanned(env);
  while (scan_unscanned_objects());
  move_unreached_to_free();
  coalesce_free_nodes();

  heap_info_t *after = get_heap_info();
  printf("GC freed %zu bytes. Bytes avail: %zu.\n", used_before - after->bytes_used, after->bytes_free);
}

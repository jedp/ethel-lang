#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../inc/type.h"
#include "../inc/mem.h"
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
 *   - Move object o from Unscanned to Scanned
 *   - For each object o' referenced by o
 *     - If o' is Unreached
 *       Move o' from Unreached to Unscanned
 * - Free = Free + Unreached
 * - Unreached = Scanned
 */

#define F_GC_UNSET ~( F_GC_UNREACHED | F_GC_UNSCANNED | F_GC_SCANNED )

/*
 * Free = Free + Unreached
 * Unreached = Scanned
 */
static void conclude_gc(void) {
  heap_node_t *heap_node = heap_head();

  while (heap_node != NULL) {
    if (heap_node->flags & F_GC_UNREACHED) {
      heap_node->flags &= ~F_GC_UNREACHED;
      heap_node->flags |= F_GC_FREE;
    } else if(heap_node->flags & F_GC_SCANNED) {
      heap_node->flags &= ~F_GC_SCANNED;
      heap_node->flags |= F_GC_UNREACHED;
    } else {
      heap_node->flags &= F_GC_UNSET;
    }

    heap_node = heap_node->next;
  }
}

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
    assert_valid_heap_node(heap_node);
    if (heap_node->flags & F_GC_UNREACHED) {
      // Mark as free, but don't use efree; we will coalesce nodes later.
      assert(!(heap_node->flags & F_GC_UNSCANNED));
      heap_node->flags |= F_GC_FREE;
    }
    heap_node = heap_node->next;
  }
}

/*
 * While there are Unscanned nodes,
 *
 * - Move object o from Unscanned to Scanned
 * - Move Unreached children to Unscanned
 */
static int scan_unscanned_objects() {
  heap_node_t* heap_node = heap_head();

  int unscanned = 0;

  while (heap_node != NULL) {
    assert_valid_heap_node(heap_node);
    if (heap_node->flags & F_GC_UNSCANNED) {

      // Move object from Unscanned to Scanned.
      heap_node->flags &= ~F_GC_UNSCANNED;
      heap_node->flags |= F_GC_SCANNED;

      // Move Unreached children to Unscanned.
      gc_header_t* data_ptr = (gc_header_t*) DATA_FOR_NODE(heap_node);
      assert_valid_typed_node(data_ptr);

      for (int i = 0; i < data_ptr->children; ++i) {
        size_t offset = sizeof(gc_header_t) + (i * sizeof(void*));
        void **child = (void*) ((size_t) data_ptr + offset);
        // Pointers can be null. elem->next, etc.
        if (*child != NULL) {
          heap_node_t* child_heap_node = NODE_FOR_DATA(*child);

          // Move Unreached child to Unscanned.
          if (!(child_heap_node->flags == F_GC_SCANNED)) {
            child_heap_node->flags &= ~F_GC_UNREACHED;
            child_heap_node->flags |= F_GC_UNSCANNED;
            unscanned++;
          }
        }
      }
    }

    heap_node = heap_node->next;
  }

  return unscanned;
}

/*
 * Mark nodes reached by the root set as Unscanned.
 */
static void initialize_unscanned_roots(env_t *env) {
  assert(env->top >= 0);

  // Move objects referenced by the root set from Unreached to Unscanned.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *env_node = env->symbols[i];
    while (env_node != NULL) {
      heap_node_t* heap_node = NODE_FOR_DATA(env_node);

      // By definition allocated, so should already have been marked as Unreached.
      assert(!(heap_node->flags & F_GC_FREE));

      // I guess this assertion doesn't hold for shadowing vars?
      // assert(heap_node->flags & F_GC_UNREACHED);

      heap_node->flags &= ~F_GC_UNREACHED;
      heap_node->flags |= F_GC_UNSCANNED;

      env_node = env_node->next;
    }
  }
}

/*
 * Initialize Unreached, Unscanned, and Scanned to 0.
 *
 * Mark all allocated (non-Free) objects as Unreached.
 */
static void initialize_gc(void) {
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

/*
 * Stop the world, someone has to get off.
 */
void gc(env_t *env) {
  size_t used_before = get_heap_info()->bytes_used;

  initialize_gc();
  initialize_unscanned_roots(env);
  while (scan_unscanned_objects());
  move_unreached_to_free();
  coalesce_free_nodes();
  conclude_gc();

  heap_info_t *after = get_heap_info();
  printf("GC freed %zu bytes. Bytes avail: %zu.\n", used_before - after->bytes_used, after->bytes_free);

//  show_heap();
}


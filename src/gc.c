#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
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

#define F_GC_UNSET ~( F_GC_UNREACHED | F_GC_UNSCANNED | F_GC_SCANNED )

static void coalesce_free_nodes(void) {
  heap_node_t *heap_node = heap_head();

  // Starting at the beginning of the heap, find the next free node and
  // absorb all subsequent free nodes into it. Repeat until there are no
  // more nodes.
  while (heap_node != NULL) {
    if (heap_node->flags & F_FREE) {
      while (heap_node->next != NULL && (heap_node->next->flags & F_FREE)) {
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
      heap_node->flags = F_FREE;
    }
    heap_node = heap_node->next;
  }
}

static void move_unreached_to_unscanned(void *obj) {
  if (NODE_FOR_DATA(obj)->flags & F_GC_UNREACHED) {
    printf("move %p from unreached to unscanned\n", obj);
    NODE_FOR_DATA(obj)->flags &= ~(F_GC_UNREACHED);
    NODE_FOR_DATA(obj)->flags |= F_GC_UNSCANNED;
  }
}

static void move_unreached_descendants_to_unscanned(obj_t *obj) {
  printf("mark %s obj and descendants\n", obj_type_names[obj->type]);

  switch (obj->type) {
    case TYPE_BYTEARRAY:
    case TYPE_STRING:
      move_unreached_to_unscanned(obj->bytearray);
      break;
    case TYPE_LIST: {
      // Lists contain *obj and *elems.
      move_unreached_to_unscanned(obj->list);
      obj_list_element_t *next = obj->list->elems;
      while (next != NULL) {
        move_unreached_descendants_to_unscanned(next->node);
        next = next->next;
      }
      break;
                    }
    case TYPE_DICT: {
      // Dicts contain **nodes,
      // And nodes contain *k, *v, and *next
      // k has to be a primitive type. v can be anything.
      move_unreached_to_unscanned(obj->dict);
      for (int i = 0; i < obj->dict->buckets; i++) {
        dict_node_t *kv = obj->dict->nodes[i];
        while (kv != NULL) {
          move_unreached_to_unscanned(kv->k);
          move_unreached_descendants_to_unscanned(kv->v);
          kv = kv->next;
        }
      }
      break;
                    }
    case TYPE_FUNC_PTR:
      move_unreached_to_unscanned(obj->func_def);
      break;
    case TYPE_ITERATOR:
      move_unreached_to_unscanned(obj->iterator);
      break;
    default:
      break;
  }
}

static int scan_unscanned_objects() {
  heap_node_t* heap_node = heap_head();

  int unscanned = 0;

  while (heap_node != NULL) {
    if (heap_node->flags & F_GC_UNSCANNED) {
      heap_node->flags &= ~F_GC_UNSCANNED;
      heap_node->flags |= F_GC_SCANNED;

      unscanned += 1;

      move_unreached_descendants_to_unscanned(DATA_FOR_NODE(heap_node));
    }

    heap_node = heap_node->next;
  }

  return unscanned;
}

void move_root_unreached_to_unscanned(env_t* env) {
  assert(env->top >= 0);

  // Move objects referenced by the root set from Unreached to Unscanned.
  for (int i = env->top; i >= 0; --i) {
    env_sym_t *env_node = env->symbols[i].next;
    while (env_node != NULL) {
      move_unreached_to_unscanned(env_node->obj);
      env_node = env_node->next;
    }
  }
}

static void prepare_new_scan(env_t *env) {
  heap_node_t* heap_node = heap_head();
  while (heap_node != NULL) {

    // Scanned = Unscanned = 0.
    heap_node->flags &= F_GC_UNSET;

    // All allocated objects are Unreached.
    if (!(heap_node->flags & F_FREE)) {
      heap_node->flags |= F_GC_UNREACHED;
    }

    heap_node = heap_node->next;
  }
}

void gc(env_t *env) {
  prepare_new_scan(env);
  move_root_unreached_to_unscanned(env);
  while (scan_unscanned_objects());
  move_unreached_to_free();
  coalesce_free_nodes();
}


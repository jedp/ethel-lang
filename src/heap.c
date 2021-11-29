#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../inc/ptr.h"
#include "../inc/heap.h"

// Ye olde heape.
static uint32_t heap[HEAP_BYTES] = { 0 };

#define HEAP_DATA_BEGIN ((size_t) heap + sizeof(heap_node_t))
#define HEAP_DATA_END ((size_t) heap + HEAP_BYTES)
#define DATA_FOR_NODE(node) ((void*) ((size_t) node + sizeof(heap_node_t)))
#define NODE_FOR_DATA(data_ptr) ((heap_node_t*) ((size_t) data_ptr - sizeof(heap_node_t)))

// A global template we will use to construct new node data
// before copying it to memory.
heap_node_t node_template = {
  .prev = NULL,
  .next = NULL,
  .flags = 42   // Weird flags as an error marker.
};

// Heap info is statically allocated so it won't interfere with the heap.
heap_info_t heap_info = {
  .total_nodes = 0,
  .free_nodes = 0,
  .bytes_free = HEAP_BYTES
};

static uint32_t node_size(heap_node_t *node) {
  if (node->next == NULL) {
    return (uint32_t) (HEAP_DATA_END - (size_t) node) - sizeof(heap_node_t);
  }

  return (uint32_t) ((size_t) node->next - (size_t) node) - sizeof(heap_node_t);
}

/*
 * Take a piece of a node at the specified new size. If the new size
 * is smaller than the current node, break it into two nodes.
 *
 * The second node may have a byte size of 0. This is so it can be
 * reclaimed through coalescence with an adjacent free node.
 *
 * All allocations on the heap are ultimately derived by fracturing
 * the initial node that contains all the free bytes.
 *
 * Mutates the properties of the node, but leaves flags untouched.
 */
static void fracture_node(heap_node_t *node, uint32_t new_size) {
  assert(new_size >= 0);
  uint32_t size = node_size(node);
  assert(new_size <= size);
  size_t remaining = size - new_size;

  // If too small to fracture, do nothing.
  if (remaining < sizeof(heap_node_t)) return;

  // Create a new, smaller node with the remainder.
  // Everything should still be a multiple of heap node size.
  assert(remaining % sizeof(heap_node_t) == 0);

  // Create new node.
  node_template.prev = NULL;
  node_template.next = NULL;
  node_template.flags = F_FREE; // New node is by definition unused.

  // Jam it into memory and get a pointer to it.
  size_t new_location = (size_t) node + new_size + sizeof(heap_node_t);
  heap_node_t *new_node = (heap_node_t*) new_location;
  mem_cp(new_node, &node_template, sizeof(heap_node_t));

  // Re-wire everything.
  new_node->prev = node;
  new_node->next = node->next;
  if (node->next != NULL) {
    node->next->prev = new_node;
  }
  node->next = new_node;
}

/*
 * If the two nodes are both free, coalesce them into a single
 * free node.
 */
static void coalesce_nodes(heap_node_t *left,
                           heap_node_t *right) {
  if (left == NULL || right == NULL) return;
  if (!((left->flags & right->flags) & F_FREE)) return;

  // Combine sizes and absorb the header bytes.
  left->next = right->next;
  if (left->next != NULL) {
    left->next->prev = left;
  }

  // Null out the original right-side node for safety.
  right->prev = NULL;
  right->next = NULL;
  right->flags = 42;
}

void *ealloc(uint32_t bytes) {
  if (bytes == 0) return NULL;

  // We allocate things in heap-node-size blocks.
  if (bytes % sizeof(heap_node_t) != 0) {
    bytes += sizeof(heap_node_t) - (bytes % sizeof(heap_node_t));
  }

  // Find the first free node of sufficient size.
  heap_node_t *node = (heap_node_t*) heap;
  while(node != NULL) {
    if (bytes <= node_size(node) && (node->flags & F_FREE)) break;
    node = node->next;
  }

  if (node == NULL) {
    printf("Out of heap space!\n");
    dump_heap();
    return NULL;
  }

  // Out of memory :(
  if (node_size(node) < bytes || !(node->flags & F_FREE)) {
    printf("No nodes with sufficient capacity. Out of memory!\n");
    dump_heap();
    return NULL;
  }

  // Update header on this node. This is what we will return.
  fracture_node(node, bytes);
  node->flags &= ~F_FREE;

  // Return pointer to the data buffer.
  return DATA_FOR_NODE(node);
}

void *erealloc(void* data_ptr, uint32_t size) {
  assert(size >= 0);

  // If null and no size, return a minimal allocation.
  if (data_ptr == NULL && size == 0) return ealloc(1);

  // If null, behaves as ealloc(size);
  if (data_ptr == NULL) return ealloc(size);

  // Get the heap node associated with this pointer.
  heap_node_t *node = NODE_FOR_DATA(data_ptr);

  // Change the allocation if bytes is smaller than existing node.
  if (size < node_size(node)) {
    fracture_node(node, size);
    return node;
  }

  // Try to allocate a bigger space for it. This may fail and return NULL.
  heap_node_t *new_node = ealloc(size);
  if (new_node == NULL) return NULL;

  void *new_ptr = DATA_FOR_NODE(new_node);
  mem_cp(data_ptr, new_ptr, node_size(node));

  // Move the flags from src to dst.
  new_node->flags = node->flags;
  node->flags = F_FREE;

  // TODO: NULL out data_ptr or not?
  return new_ptr;
}

void efree(void *data_ptr) {
  assert((size_t) data_ptr >= HEAP_DATA_BEGIN);
  assert((size_t) data_ptr <= HEAP_DATA_END);

  // Pointer arithmetic to find metadata for node.
  heap_node_t *node = NODE_FOR_DATA(data_ptr);
  assert(((size_t) node - (size_t) heap) % sizeof(heap_node_t) == 0);

  // Mark as free.
  node->flags |= F_FREE;

  // Merge adjacent free nodes. The order matters.
  coalesce_nodes(node, node->next);
  coalesce_nodes(node->prev, node);

  data_ptr = NULL;
}

/*
 * Initialize the heap once and for all.
 *
 * Exposed for testing.
 */
void heap_init(void) {
  // Create a node containing the entire heap.
  node_template.prev = NULL;
  node_template.next = NULL;
  node_template.flags = F_FREE;
  mem_cp(heap, &node_template, sizeof(heap_node_t));
}

heap_info_t *get_heap_info() {
  heap_info.total_nodes = 0;
  heap_info.free_nodes = 0;
  heap_info.bytes_used = 0;
  heap_info.bytes_free = 0;

  heap_node_t *node = (heap_node_t*) heap;

  while (node != NULL) {
    heap_info.total_nodes++;
    if (node->flags & F_FREE) {
      heap_info.free_nodes++;
      heap_info.bytes_free += node_size(node);
    } else {
      heap_info.bytes_used += node_size(node);
    }
    node = node->next;
  }

  return &heap_info;
}

void dump_heap(void) {
  printf("Heap start: 0x%lx\n", HEAP_DATA_BEGIN);
  printf("Heap end:   0x%lx\n", HEAP_DATA_END);
  printf("Heap bytes: %zu\n", HEAP_BYTES);
  get_heap_info();
  printf("==== Heap\n");
  printf("  Total nodes: %d\n", heap_info.total_nodes);
  printf("   Free nodes: %d\n", heap_info.free_nodes);
  printf("   Bytes used: %d\n", heap_info.bytes_used);
  printf("   Bytes free: %d\n", heap_info.bytes_free);
}


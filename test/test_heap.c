#include "unity/unity.h"
#include "test_heap.h"
#include "../inc/heap.h"

#define BLOCK_SIZE ((uint32_t) sizeof(heap_node_t))
#define HEAP_MAX ((uint32_t) HEAP_BYTES - BLOCK_SIZE)

void test_heap_init(void) {
  heap_init();

  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(1, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(0, heap->bytes_used);
  TEST_ASSERT_EQUAL(HEAP_MAX, heap->bytes_free);
}

void test_heap_alloc(void) {
  heap_init();

  // This will require BLOCK_SIZE bytes.
  ealloc(BLOCK_SIZE - 5);
  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(2, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(BLOCK_SIZE, heap->bytes_used);

  // This will require BLOCK_SIZE + BLOCK_SIZE bytes.
  ealloc(BLOCK_SIZE + 1);
  heap = get_heap_info();
  TEST_ASSERT_EQUAL(3, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(3 * BLOCK_SIZE, heap->bytes_used);
}

void test_heap_free(void) {
  heap_init();

  // Allocate a node.
  void *p1 = ealloc(BLOCK_SIZE);
  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(2, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(BLOCK_SIZE, heap->bytes_used);
  TEST_ASSERT_NOT_EQUAL(HEAP_MAX, heap->bytes_free);

  // Free the node and return to original state.
  efree(p1);
  heap = get_heap_info();
  TEST_ASSERT_EQUAL(1, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(0, heap->bytes_used);
  TEST_ASSERT_EQUAL(HEAP_MAX, heap->bytes_free);
}

void test_heap_free_then_fill_hole(void) {
  heap_init();

  ealloc(BLOCK_SIZE);
  void *p = ealloc(BLOCK_SIZE + 1); // req 2 blocks
  ealloc(BLOCK_SIZE);

  heap_info_t *heap = get_heap_info();
  uint32_t size = heap->bytes_free;
  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);

  // Freeing the second block leaves a 2 block hole.
  efree(p);

  // Allocating two more blocks fills in the hole.
  ealloc(BLOCK_SIZE + 1);
  heap = get_heap_info();

  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(size, heap->bytes_free);
}

void test_heap_free_then_partially_fill_hole(void) {
  heap_init();

  ealloc(BLOCK_SIZE);
  void *p = ealloc(BLOCK_SIZE + 1); // req 2 blocks
  ealloc(BLOCK_SIZE);

  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);

  // Freeing the second block leaves a 2 block hole.
  efree(p);

  // Allocate a single block to fragment the hole.
  // This will allocate a single node and create a one-block node.
  ealloc(BLOCK_SIZE);
  heap = get_heap_info();

  // Three nodes have been allocated.
  // Two nodes are free: One small one (fragmentation), and the rest.
  TEST_ASSERT_EQUAL(5, heap->total_nodes);
  TEST_ASSERT_EQUAL(2, heap->free_nodes);
}

void test_heap_free_and_coalesce_left(void) {
  heap_init();

  ealloc(BLOCK_SIZE);
  void *p1 = ealloc(BLOCK_SIZE);
  void *p2 = ealloc(BLOCK_SIZE);
  ealloc(BLOCK_SIZE);

  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(5, heap->total_nodes);
  uint32_t size = heap->bytes_free;

  // Free the node on the left.
  // Freeing the node on the right will coalesce the free spaces.
  efree(p1);
  efree(p2);
  heap = get_heap_info();
  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(2, heap->free_nodes);
  // Implementaton detail: There's one block for the header info.
  TEST_ASSERT_EQUAL(size + 3 * BLOCK_SIZE, heap->bytes_free);
}

void test_heap_free_and_coalesce_right(void) {
  heap_init();

  ealloc(BLOCK_SIZE);
  void *p1 = ealloc(BLOCK_SIZE);
  void *p2 = ealloc(BLOCK_SIZE);
  ealloc(BLOCK_SIZE);

  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(5, heap->total_nodes);
  uint32_t size = heap->bytes_free;

  // Free the node on the right.
  // Freeing the node on the left will coalesce the free spaces.
  efree(p2);
  efree(p1);
  heap = get_heap_info();
  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(2, heap->free_nodes);
  // Implementaton detail: There's one block for the header info.
  TEST_ASSERT_EQUAL(size + 3 * BLOCK_SIZE, heap->bytes_free);
}

void test_heap_free_and_coalesce_three(void) {
  heap_init();

  ealloc(BLOCK_SIZE);
  void *p1 = ealloc(BLOCK_SIZE);
  void *p2 = ealloc(BLOCK_SIZE);
  void *p3 = ealloc(BLOCK_SIZE);
  ealloc(BLOCK_SIZE);

  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(6, heap->total_nodes);
  uint32_t size = heap->bytes_free;

  // Free the nodes on the right and left.
  // Freeing the node in the center will coalesce the free spaces.
  efree(p1);
  efree(p3);
  efree(p2);
  heap = get_heap_info();
  TEST_ASSERT_EQUAL(4, heap->total_nodes);
  TEST_ASSERT_EQUAL(2, heap->free_nodes);
  // Implementaton detail: There's one block for the header info.
  // Each of p1, p2, and p3 required two blocks total.
  // The resulting empty space now is header + 5 blocks.
  TEST_ASSERT_EQUAL(size + 5 * BLOCK_SIZE, heap->bytes_free);
}

void test_heap_free_and_coalesce_everything(void) {
  heap_init();

  void *p1 = ealloc(BLOCK_SIZE);
  void *p2 = ealloc(BLOCK_SIZE * 2);
  void *p3 = ealloc(BLOCK_SIZE * 4);
  void *p4 = ealloc(BLOCK_SIZE * 6);
  void *p5 = ealloc(BLOCK_SIZE * 8);
  void *p6 = ealloc(BLOCK_SIZE * 10);

  efree(p2);
  efree(p4);
  efree(p6);
  efree(p1);
  efree(p3);
  efree(p5);
  heap_info_t *heap = get_heap_info();
  TEST_ASSERT_EQUAL(1, heap->total_nodes);
  TEST_ASSERT_EQUAL(1, heap->free_nodes);
  TEST_ASSERT_EQUAL(HEAP_MAX, heap->bytes_free);
}

void test_heap(void) {
  RUN_TEST(test_heap_init);
  RUN_TEST(test_heap_alloc);
  RUN_TEST(test_heap_free);
  RUN_TEST(test_heap_free_then_fill_hole);
  RUN_TEST(test_heap_free_then_partially_fill_hole);
  RUN_TEST(test_heap_free_and_coalesce_left);
  RUN_TEST(test_heap_free_and_coalesce_right);
  RUN_TEST(test_heap_free_and_coalesce_three);
  RUN_TEST(test_heap_free_and_coalesce_everything);
}


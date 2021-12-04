#include <stdio.h>
#include <stdarg.h>
#include "util.h"
#include "unity/unity.h"
#include "test_list.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/list.h"
#include "../inc/obj.h"

void test_list_len(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(0, list_len(list, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
}

void test_list_get(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(TYPE_NIL, list_get(list, n_args(1, 0))->type);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_get(list, n_args(1, 2))->intval);
}

void test_list_slice(void) {
  obj_t *list = make_list(0);
  obj_t *slice = list_slice(list, n_args(2, 0, 2));
  TEST_ASSERT_EQUAL(0, list_len(slice, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  slice = list_slice(list, n_args(2, 0, 2));
  TEST_ASSERT_EQUAL(1, slice->list->elems->node->intval);
  TEST_ASSERT_EQUAL(2, slice->list->elems->next->node->intval);
  TEST_ASSERT_EQUAL(2, list_len(slice, NULL)->intval);
}

void test_list_contains(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(False, list_contains(list, n_args(1, 42))->boolval);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(True, list_contains(list, n_args(1, 2))->boolval);
}

void test_list_head(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(TYPE_NIL, list_head(list, NULL)->type);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(1, list_head(list, NULL)->intval);
}

void test_list_tail(void) {
  obj_t *list = make_list(0);
  obj_t *slice = list_tail(list, NULL);
  TEST_ASSERT_EQUAL(0, list_len(slice, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  slice = list_tail(list, NULL);
  TEST_ASSERT_EQUAL(2, slice->list->elems->node->intval);
  TEST_ASSERT_EQUAL(3, slice->list->elems->next->node->intval);
  TEST_ASSERT_EQUAL(2, list_len(slice, NULL)->intval);
}

void test_list_prepend(void) {
  obj_t *list = make_list(0);
  list_prepend(list, n_args(1, 42));
  TEST_ASSERT_EQUAL(42, list_head(list, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  list_prepend(list, n_args(1, 42));
  TEST_ASSERT_EQUAL(42, list_head(list, NULL)->intval);
  TEST_ASSERT_EQUAL(3, list_get(list, n_args(1, 3))->intval);
}

void test_list_append(void) {
  obj_t *list = make_list(0);
  list_append(list, n_args(1, 42));
  TEST_ASSERT_EQUAL(42, list_head(list, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  list_append(list, n_args(1, 42));
  TEST_ASSERT_EQUAL(1, list_head(list, NULL)->intval);
  TEST_ASSERT_EQUAL(42, list_get(list, n_args(1, 3))->intval);
}

void test_list_remove_first(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(TYPE_NIL, list_remove_first(list, NULL)->type);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
  TEST_ASSERT_EQUAL(1, list_remove_first(list, NULL)->intval);
  TEST_ASSERT_EQUAL(2, list_len(list, NULL)->intval);
}

void test_list_remove_last(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(TYPE_NIL, list_remove_last(list, NULL)->type);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
  TEST_ASSERT_EQUAL(3, list_remove_last(list, NULL)->intval);
  TEST_ASSERT_EQUAL(2, list_len(list, NULL)->intval);
}

void test_list_remove_at(void) {
  obj_t *list = make_list(0);
  TEST_ASSERT_EQUAL(TYPE_NIL, list_remove_at(list, n_args(1, 1))->type);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
  TEST_ASSERT_EQUAL(1, list_remove_at(list, n_args(1, 0))->intval);
  TEST_ASSERT_EQUAL(2, list_head(list, NULL)->intval);
  TEST_ASSERT_EQUAL(2, list_get(list, n_args(1, 0))->intval);
  TEST_ASSERT_EQUAL(3, list_get(list, n_args(1, 1))->intval);
  TEST_ASSERT_EQUAL(2, list_len(list, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
  TEST_ASSERT_EQUAL(2, list_remove_at(list, n_args(1, 1))->intval);
  TEST_ASSERT_EQUAL(1, list_head(list, NULL)->intval);
  TEST_ASSERT_EQUAL(1, list_get(list, n_args(1, 0))->intval);
  TEST_ASSERT_EQUAL(3, list_get(list, n_args(1, 1))->intval);
  TEST_ASSERT_EQUAL(2, list_len(list, NULL)->intval);

  list = make_list(3, 1, 2, 3);
  TEST_ASSERT_EQUAL(3, list_len(list, NULL)->intval);
  TEST_ASSERT_EQUAL(3, list_remove_at(list, n_args(1, 2))->intval);
  TEST_ASSERT_EQUAL(1, list_head(list, NULL)->intval);
  TEST_ASSERT_EQUAL(1, list_get(list, n_args(1, 0))->intval);
  TEST_ASSERT_EQUAL(2, list_get(list, n_args(1, 1))->intval);
  TEST_ASSERT_EQUAL(2, list_len(list, NULL)->intval);
}

void test_list(void) {
  RUN_TEST(test_list_len);
  RUN_TEST(test_list_get);
  RUN_TEST(test_list_slice);
  RUN_TEST(test_list_contains);
  RUN_TEST(test_list_head);
  RUN_TEST(test_list_tail);
  RUN_TEST(test_list_prepend);
  RUN_TEST(test_list_append);
  RUN_TEST(test_list_remove_first);
  RUN_TEST(test_list_remove_last);
  RUN_TEST(test_list_remove_at);
}

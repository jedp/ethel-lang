#include <stdio.h>
#include <stdarg.h>
#include "unity/unity.h"
#include "test_list.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/list.h"
#include "../inc/obj.h"

static obj_method_args_t *n_args(int n, ...) {
  va_list vargs;
  va_start(vargs, n);

  obj_method_args_t *args = mem_alloc(sizeof(obj_method_args_t));
  obj_method_args_t *root = args;

  for (int i = 0; i < n; i++) {
    int val = va_arg(vargs, int);
    args->arg = int_obj(val);

    if (i < n - 1) {
      args->next = mem_alloc(sizeof(obj_method_args_t));
    } else {
      args->next = NULL;
    }

    args = args->next;
  }

  return root;
}

static obj_t *make_list(int n_elems, ...) {
  if (n_elems == 0) {
    return list_obj(NULL);
  }

  obj_list_element_t *elem = mem_alloc(sizeof(obj_list_element_t));
  obj_list_element_t *root_elem = elem;

  elem->node = NULL;
  elem->next = NULL;

  va_list vargs;
  va_start(vargs, n_elems);

  for (int i = 0; i < n_elems; i++) {
    int val = va_arg(vargs, int);
    elem->node = int_obj(val);
    if (i < n_elems - 1) {
      elem->next = mem_alloc(sizeof(obj_list_element_t));
    } else {
      elem->next = NULL;
    }

    elem = elem->next;
  }

  va_end(vargs);

  return list_obj(root_elem);
}

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

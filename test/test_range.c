#include <stdio.h>
#include <stdarg.h>
#include "unity/unity.h"
#include "test_range.h"
#include "../inc/range.h"
#include "../inc/obj.h"

void test_minimal_range(void) {
  obj_t *obj = range_obj(1, 1);
  TEST_ASSERT_EQUAL(1, range_get(obj, wrap_varargs(1, int_obj(0)))->intval);
  TEST_ASSERT_EQUAL(1, range_length(obj, NULL)->intval);
  TEST_ASSERT_EQUAL(TYPE_NIL, range_get(obj, wrap_varargs(1, int_obj(4)))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, range_get(obj, wrap_varargs(1, int_obj(4)))->type);
}

void test_range_get(void) {
  obj_t *obj = range_obj(1, 5);
  TEST_ASSERT_EQUAL(1, range_get(obj, wrap_varargs(1, int_obj(0)))->intval);
  TEST_ASSERT_EQUAL(5, range_get(obj, wrap_varargs(1, int_obj(4)))->intval);

  obj = range_obj(-5, -1);
  TEST_ASSERT_EQUAL(-5, range_get(obj, wrap_varargs(1, int_obj(0)))->intval);
  TEST_ASSERT_EQUAL(-1, range_get(obj, wrap_varargs(1, int_obj(4)))->intval);
}

void test_range_get_downto(void) {
  obj_t *obj = range_obj(5, 1);
  TEST_ASSERT_EQUAL(5, range_get(obj, wrap_varargs(1, int_obj(0)))->intval);
  TEST_ASSERT_EQUAL(1, range_get(obj, wrap_varargs(1, int_obj(4)))->intval);

  obj = range_obj(0, -4);
  TEST_ASSERT_EQUAL(0, range_get(obj, wrap_varargs(1, int_obj(0)))->intval);
  TEST_ASSERT_EQUAL(-4, range_get(obj, wrap_varargs(1, int_obj(4)))->intval);
}

void test_range_contains(void) {
  obj_t *obj = range_obj(1, 5);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(0)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(1)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(2)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(5)))->boolval);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(6)))->boolval);
}

void test_range_contains_downto(void) {
  obj_t *obj = range_obj(5, -1);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(0)))->boolval);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(6)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(5)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(-1)))->boolval);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(-2)))->boolval);
}

void test_range(void) {
  RUN_TEST(test_minimal_range);
  RUN_TEST(test_range_get);
  RUN_TEST(test_range_get_downto);
  RUN_TEST(test_range_contains);
  RUN_TEST(test_range_contains_downto);
}

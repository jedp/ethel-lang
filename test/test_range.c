#include <stdio.h>
#include <stdarg.h>
#include "unity/unity.h"
#include "test_range.h"
#include "../inc/range.h"
#include "../inc/obj.h"

void test_range_contains(void) {
  obj_t *obj = range_obj(1, 5);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(0)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(1)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(2)))->boolval);
  TEST_ASSERT_EQUAL(True, range_contains(obj, wrap_varargs(1, int_obj(5)))->boolval);
  TEST_ASSERT_EQUAL(False, range_contains(obj, wrap_varargs(1, int_obj(6)))->boolval);
}

void test_range(void) {
  RUN_TEST(test_range_contains);
}

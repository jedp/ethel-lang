#include "unity/unity.h"
#include "util.h"
#include "test_trace.h"
#include "../inc/obj.h"
#include "../inc/str.h"

void test_traceable_primitive(void) {
  obj_t *obj = int_obj(42);
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);
}

void test_traceable_bytearray(void) {
  obj_t *obj = bytearray_obj(0, NULL);
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((traceable_obj_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((traceable_obj_t*) obj->bytearray)->flags);
}

void test_traceable_string(void) {
  obj_t *obj = string_obj(c_str_to_bytearray("Hi!"));
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((traceable_obj_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((traceable_obj_t*) obj->bytearray)->flags);
}

void test_trace(void) {
  RUN_TEST(test_traceable_primitive);
  RUN_TEST(test_traceable_bytearray);
  RUN_TEST(test_traceable_string);
}


#include "unity/unity.h"
#include "test_bytearray.h"
#include "../inc/obj.h"

uint8_t int_arr[3] = { 1, 2, 3 };

void test_barr_new(void) {
  obj_t *a = bytearray_obj(5, (uint8_t *) "ohai");

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, a->type);
  TEST_ASSERT_EQUAL(5, a->bytearray->size);
  TEST_ASSERT_EQUAL('o', a->bytearray->data[0]);
  TEST_ASSERT_EQUAL('h', a->bytearray->data[1]);
  TEST_ASSERT_EQUAL('a', a->bytearray->data[2]);
  TEST_ASSERT_EQUAL('i', a->bytearray->data[3]);
  TEST_ASSERT_EQUAL('\0', a->bytearray->data[4]);

  obj_t *a2 = bytearray_obj(3, int_arr);
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, a2->type);
  TEST_ASSERT_EQUAL(3, a2->bytearray->size);
  TEST_ASSERT_EQUAL(1, a2->bytearray->data[0]);
  TEST_ASSERT_EQUAL(2, a2->bytearray->data[1]);
  TEST_ASSERT_EQUAL(3, a2->bytearray->data[2]);
}

void test_bytearray(void) {
  RUN_TEST(test_barr_new);
}

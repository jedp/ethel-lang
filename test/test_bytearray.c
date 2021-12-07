#include "unity/unity.h"
#include "test_bytearray.h"
#include "../inc/type.h"
#include "../inc/arr.h"
#include "../inc/obj.h"

uint8_t int_arr[3] = { 1, 2, 3 };

void test_barr_new(void) {
  obj_t *a = bytearray_obj(6, (uint8_t *) "ohai");

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, TYPEOF(a));
  TEST_ASSERT_EQUAL(6, a->bytearray->size);
  TEST_ASSERT_EQUAL('o', a->bytearray->data[0]);
  TEST_ASSERT_EQUAL('h', a->bytearray->data[1]);
  TEST_ASSERT_EQUAL('a', a->bytearray->data[2]);
  TEST_ASSERT_EQUAL('i', a->bytearray->data[3]);
  // Zero fill.
  TEST_ASSERT_EQUAL('\0', a->bytearray->data[4]);
  TEST_ASSERT_EQUAL('\0', a->bytearray->data[4]);

  obj_t *a2 = bytearray_obj(3, int_arr);
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, TYPEOF(a2));
  TEST_ASSERT_EQUAL(3, a2->bytearray->size);
  TEST_ASSERT_EQUAL(1, a2->bytearray->data[0]);
  TEST_ASSERT_EQUAL(2, a2->bytearray->data[1]);
  TEST_ASSERT_EQUAL(3, a2->bytearray->data[2]);
}

void test_barr_contains(void) {
  obj_t *a = bytearray_obj(4, (uint8_t *) "ohai");
  TEST_ASSERT_EQUAL(True, arr_contains(a, wrap_varargs(1, byte_obj('h')))->boolval);
}

void test_barr_eq(void) {
  obj_t *a = bytearray_obj(4, (uint8_t *) "ohai");
  obj_t *b = bytearray_obj(4, (uint8_t *) "ohai");
  TEST_ASSERT_EQUAL(True, arr_eq(a, wrap_varargs(1, b))->boolval);
}

void test_barr_ne(void) {
  obj_t *a = bytearray_obj(4, (uint8_t *) "ohai");
  obj_t *b = bytearray_obj(4, (uint8_t *) "glug");
  TEST_ASSERT_EQUAL(True, arr_ne(a, wrap_varargs(1, b))->boolval);
}

void test_barr_slice(void) {
  obj_t *a = bytearray_obj(4, (uint8_t *) "ohai");
  obj_t *slice = arr_slice(a, wrap_varargs(2, int_obj(0), int_obj(10)));
  TEST_ASSERT_EQUAL(4, slice->bytearray->size);
  TEST_ASSERT_EQUAL('o', slice->bytearray->data[0]);
  TEST_ASSERT_EQUAL('h', slice->bytearray->data[1]);
  TEST_ASSERT_EQUAL('a', slice->bytearray->data[2]);
  TEST_ASSERT_EQUAL('i', slice->bytearray->data[3]);

  slice = arr_slice(a, wrap_varargs(2, int_obj(0), int_obj(0)));
  TEST_ASSERT_EQUAL(0, slice->bytearray->size);

  slice = arr_slice(a, wrap_varargs(2, int_obj(0), int_obj(2)));
  TEST_ASSERT_EQUAL(2, slice->bytearray->size);
  TEST_ASSERT_EQUAL('o', slice->bytearray->data[0]);
  TEST_ASSERT_EQUAL('h', slice->bytearray->data[1]);

  slice = arr_slice(a, wrap_varargs(2, int_obj(2), int_obj(4)));
  TEST_ASSERT_EQUAL(2, slice->bytearray->size);
  TEST_ASSERT_EQUAL('a', slice->bytearray->data[0]);
  TEST_ASSERT_EQUAL('i', slice->bytearray->data[1]);
}

void test_bytearray(void) {
  RUN_TEST(test_barr_new);
  RUN_TEST(test_barr_contains);
  RUN_TEST(test_barr_eq);
  RUN_TEST(test_barr_ne);
  RUN_TEST(test_barr_slice);
}


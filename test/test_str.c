#include "unity/unity.h"
#include "test_str.h"
#include "../inc/mem.h"
#include "../inc/str.h"

void test_c_str_len(void) {
  TEST_ASSERT_EQUAL(0, c_str_len(""));
  TEST_ASSERT_EQUAL(1, c_str_len("."));
  TEST_ASSERT_EQUAL(10, c_str_len("I like pie"));
}

void test_c_str_eq(void) {
  TEST_ASSERT_EQUAL(True, c_str_eq("", ""));
  TEST_ASSERT_EQUAL(True, c_str_eq(":)", ":)"));
  TEST_ASSERT_EQUAL(True, c_str_eq("I like pie!", "I like pie!"));
  TEST_ASSERT_EQUAL(False, c_str_eq("Giraffes?", "Giraffes!"));
  TEST_ASSERT_EQUAL(False, c_str_eq("", "Ethel"));
  TEST_ASSERT_EQUAL(False, c_str_eq("Ethel", ""));
  TEST_ASSERT_EQUAL(False, c_str_eq("I like pie?", "I like potatoes!"));
  TEST_ASSERT_EQUAL(False, c_str_eq("I like potatoes?", "I like pie!"));
}

void test_c_str_cp(void) {
  char* src = "I like pie!";
  char* dst = mem_alloc(12);
  c_str_cp(dst, src);

  TEST_ASSERT_EQUAL(True, c_str_eq(dst, src));
}

void test_c_str_ncat(void) {
  char* dst = mem_alloc(80);
  dst[0]= '\0';
  
  c_str_ncat(dst, "I ", 2);
  TEST_ASSERT_EQUAL(True, c_str_eq(dst, "I "));
  c_str_ncat(dst, "like pie!", 7);
  TEST_ASSERT_EQUAL(True, c_str_eq(dst, "I like pi"));
}

void test_c_str_bytearray_round_trip(void) {
  char *cstr = "I like pie";
  bytearray_t *a = c_str_to_bytearray(cstr);

  TEST_ASSERT_EQUAL_STRING(cstr, bytearray_to_c_str(a));
}

void test_str_arr_dump(void) {
  bytearray_t *a = c_str_to_bytearray("I like traffic lights.");
  char *expected = "00000000  49 20 6c 69 6b 65 20 74  72 61 66 66 69 63 20 6c  |I.like.traffic.l|\n00000010  69 67 68 74 73 2e                                 |ights.|";

  obj_t *s = arr_dump(string_obj(a));
  TEST_ASSERT_EQUAL_STRING(expected, bytearray_to_c_str(s->bytearray));
}

void test_str_int_dump(void) {
  // Negative
  char *expected1 = "0xfffffffe  11111111 11111111 11111111 11111110";
  obj_t *s1 = int32_dump(int_obj(-2));
  TEST_ASSERT_EQUAL_STRING(expected1, bytearray_to_c_str(s1->bytearray));

  // Positive
  char *expected2 = "0x0001e240  00000000 00000001 11100010 01000000";
  obj_t *s2 = int32_dump(int_obj(123456));
  TEST_ASSERT_EQUAL_STRING(expected2, bytearray_to_c_str(s2->bytearray));
}

void test_str_float_dump(void) {
  char *expected = "1  10000001  10110011001100110011010";
  obj_t *s = float32_dump(float_obj(-6.8));
  TEST_ASSERT_EQUAL_STRING(expected, bytearray_to_c_str(s->bytearray));
}

void test_str_byte_dump(void) {
  char *expected = " 99  0x63  01100011";
  obj_t *s = byte_dump(char_obj('c'));
  TEST_ASSERT_EQUAL_STRING(expected, bytearray_to_c_str(s->bytearray));
}

void test_str(void) {
  RUN_TEST(test_c_str_len);
  RUN_TEST(test_c_str_eq);
  RUN_TEST(test_c_str_cp);
  RUN_TEST(test_c_str_ncat);
  RUN_TEST(test_c_str_bytearray_round_trip);
  RUN_TEST(test_str_arr_dump);
  RUN_TEST(test_str_int_dump);
  RUN_TEST(test_str_float_dump);
  RUN_TEST(test_str_byte_dump);
}


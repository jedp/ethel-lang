#include <stdlib.h>
#include "unity/unity.h"
#include "test_str.h"
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
  char* dst = malloc(12);
  c_str_cp(dst, src);

  TEST_ASSERT_EQUAL(True, c_str_eq(dst, src));
}

void test_c_str_ncat(void) {
  char* dst = malloc(80);
  dst[0]= '\0';
  
  c_str_ncat(dst, "I ", 2);
  TEST_ASSERT_EQUAL(True, c_str_eq(dst, "I "));
  c_str_ncat(dst, "like pie!", 7);
  TEST_ASSERT_EQUAL(True, c_str_eq(dst, "I like pi"));
}

void test_str(void) {
  RUN_TEST(test_c_str_len);
  RUN_TEST(test_c_str_eq);
  RUN_TEST(test_c_str_cp);
  RUN_TEST(test_c_str_ncat);
}


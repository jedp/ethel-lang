#include "unity/unity.h"
#include "test_ptr.h"
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/str.h"

void test_mem_set(void) {
  char* s = mem_alloc(10);

  mem_set(s, 0, 10);
  TEST_ASSERT_EQUAL_STRING("", s);
  TEST_ASSERT_EQUAL(0, c_str_len(s));

  mem_set(s, (int) 'x', 5);
  TEST_ASSERT_EQUAL_STRING("xxxxx", s);
  TEST_ASSERT_EQUAL(5, c_str_len(s));
}

void test_ptr(void) {
  RUN_TEST(test_mem_set);
}


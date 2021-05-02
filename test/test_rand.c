#include "unity/unity.h"
#include "test_rand.h"
#include "../inc/rand.h"

void test_rand_seq(void) {
  // Regression test. In case this ever changes.
  rand32_init();
  TEST_ASSERT_EQUAL(127028579, rand32());
  TEST_ASSERT_EQUAL(186631676, rand32());
  TEST_ASSERT_EQUAL(3451632044, rand32());
}

void test_rand_seed(void) {
  rand32_seed(42);
  TEST_ASSERT_EQUAL(2719499560, rand32());
  TEST_ASSERT_EQUAL(2186321590, rand32());
  TEST_ASSERT_EQUAL(988862677, rand32());
}

void test_rand(void) {
  RUN_TEST(test_rand_seq);
  RUN_TEST(test_rand_seed);
}


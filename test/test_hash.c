#include <stdio.h>
#include <stdarg.h>
#include "unity/unity.h"
#include "test_hash.h"
#include "../inc/mem.h"
#include "../inc/env.h"
#include "../inc/eval.h"

static eval_result_t *eval_program(char* program) {
  env_t env;
  env_init(&env);

  enter_scope(&env);

  return eval(&env, program);

  leave_scope(&env);
}

void test_hash_values(void) {
  // Regression test.
  TEST_ASSERT_EQUAL(1698334901, eval_program("\"Ethel\".hash()")->obj->intval);
  TEST_ASSERT_EQUAL(42,         eval_program("(42).hash()")->obj->intval);
  TEST_ASSERT_EQUAL(1078523331, eval_program("3.14.hash()")->obj->intval);
  TEST_ASSERT_EQUAL(1078699117, eval_program("arr(10).hash()")->obj->intval);
  TEST_ASSERT_EQUAL(99,         eval_program("'c'.hash()")->obj->intval);
  TEST_ASSERT_EQUAL(0,          eval_program("false.hash()")->obj->intval);

  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.hash() \n"
                  "}";
  TEST_ASSERT_EQUAL(1456420779, eval_program(program)->obj->intval);
}

void test_hash(void) {
  RUN_TEST(test_hash_values);
}


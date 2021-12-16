#include <stdio.h>
#include <stdarg.h>
#include "util.h"
#include "unity/unity.h"
#include "test_hash.h"
#include "../inc/mem.h"
#include "../inc/env.h"
#include "../inc/eval.h"

static int evaluate(const char* program) {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  eval_result_t *result = mem_alloc(sizeof(eval_result_t));

  eval(&env, program, result);

  return result->obj->intval;
}

void test_hash_values(void) {
  // Regression test.
  TEST_ASSERT_EQUAL(1698334901, evaluate("\"Ethel\".hash()"));
  TEST_ASSERT_EQUAL(42,         evaluate("(42).hash()"));
  TEST_ASSERT_EQUAL(1078523331, evaluate("3.14.hash()"));
  TEST_ASSERT_EQUAL(1078699117, evaluate("arr(10).hash()"));
  TEST_ASSERT_EQUAL(99,         evaluate("'c'.hash()"));
  TEST_ASSERT_EQUAL(0,          evaluate("false.hash()"));

  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.hash() \n"
                  "}";
  TEST_ASSERT_EQUAL(1456420779, evaluate(program));
}

void test_hash(void) {
  RUN_TEST(test_hash_values);
}


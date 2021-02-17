#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/env.h"
#include "../inc/eval.h"

void test_eval_calculator(void) {
  char *program = "4 + 3 * (3+2) + -10/2";

  env_t env;
  env_init(&env);

  eval_result_t *result = eval(&env, program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(14, obj->intval);
}

void test_eval(void) {
  RUN_TEST(test_eval_calculator);
}


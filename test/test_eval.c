#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/eval.h"

void test_calculator(void) {
  char *program = "4 + 3 * (3+2) + -10/2";

  eval_result_t *result = eval(program);
  TEST_ASSERT_EQUAL(AST_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(14, result->intval);
}

void test_eval(void) {
  RUN_TEST(test_calculator);
}


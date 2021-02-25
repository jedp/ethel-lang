#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/env.h"
#include "../inc/eval.h"

eval_result_t *eval_program(char* program) {
  env_t env;
  env_init(&env);

  push_scope(&env);

  return eval(&env, program);
}

void test_eval_calculator(void) {
  char *program = "4 + 3 * (3+2) + -10/2";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(14, obj->intval);
}

void test_eval_if_else(void) {
  char *program = "if 12 then x = 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_nil(void) {
  char *program = "if 0 then 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_NIL, obj->type);
}

void test_eval_if_else_assign(void) {
  char *program = "if 12 then x = 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_assign_expr(void) {
  char *program = "x = if 12 then 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_boolean_true(void) {
  char *program = "x = 1 and (1 or 0) or (0 and 1 or 1)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_boolean_false(void) {
  char *program = "x = 0 and (1 or 0) or (0 and 1 or 0)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 0 is boolean false.
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_truthiness(void) {
  char *program = "x = (\"glug\" or 0) and (4.3 or 0) and (0 or 'c')"; 
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval(void) {
  RUN_TEST(test_eval_calculator);
  RUN_TEST(test_eval_if_else);
  RUN_TEST(test_eval_if_else_nil);
  RUN_TEST(test_eval_if_else_assign);
  RUN_TEST(test_eval_if_else_assign_expr);
  RUN_TEST(test_eval_boolean_true);
  RUN_TEST(test_eval_boolean_false);
  RUN_TEST(test_eval_truthiness);
}


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

void test_eval_assign_immutable(void) {
  char *program = "{\nval x = 32\nx = 33\n}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ENV_SYMBOL_REDEFINED, result->err);
}

void test_eval_assign_var(void) {
  char *program = "{\nvar x = 32\nx = 33\n}\n";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);
}

void test_eval_assign_multiple(void) {
  char *program = "{\nvar x = 42\nval y = 9\nx = y\nwhile (x < 12) {\nx = x + 1\n}\nx }\n";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, ((obj_t *)result->obj)->intval);
}

void test_eval_if_else(void) {
  char *program = "if 12 then val x = 5";
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
  char *program = "if 12 then val x = 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_assign_expr(void) {
  char *program = "val x = if 12 then 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_boolean_true(void) {
  char *program = "val x = 1 and (1 or 0) or (0 and 1 or 1)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_boolean_false(void) {
  char *program = "val x = 0 and (1 or 0) or (0 and 1 or 0)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 0 is boolean false.
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_truthiness(void) {
  char *program = "val x = (\"glug\" or 0) and (4.3 or 0) and (0 or 'c')"; 
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_int_mod(void) {
  char *program = "7 mod 4";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_float_mod(void) {
  char *program = "16.2 mod 3";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_FLOAT_WITHIN(0.001, 1.2, obj->floatval);
}

void test_eval_numeric_comparison(void) {
  char *program = "if 5 < 3.1 then val x = 2 else val x = if 5>= 3 then 42 else -1";

  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_char_comparison(void) {
  char *program = "if 'c' > 'd' then 'e' else if 'f' == 'f' then 'g'";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL('g', obj->charval);
}

void test_eval_cast_int(void) {
  obj_t *obj; 

  obj = eval_program("-99 as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(-99, obj->intval);

  obj = eval_program("-99 as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(-99.0, obj->floatval);

  obj = eval_program("-99 as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("-99", obj->stringval);

  obj = eval_program("99 as char")->obj;
  TEST_ASSERT_EQUAL(TYPE_CHAR, obj->type);
  TEST_ASSERT_EQUAL('c', obj->charval);

  obj = eval_program("-99 as boolean")->obj;
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, obj->type);
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_float(void) {
  obj_t *obj; 

  obj = eval_program("-42.99 as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(-42, obj->intval);

  obj = eval_program("-42.99 as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(-42.99, obj->floatval);

  obj = eval_program("-42.02 as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("-42.020000", obj->stringval);

  obj = eval_program("-42.99 as boolean")->obj;
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, obj->type);
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_string(void) {
  obj_t *obj; 

  obj = eval_program("\"-12.44\" as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(-12, obj->intval);

  obj = eval_program("\"-42.99\" as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(-42.99, obj->floatval);

  obj = eval_program("\"-42.1\" as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("-42.1", obj->stringval);

  obj = eval_program("\"-42.99\" as boolean")->obj;
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, obj->type);
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_char(void) {
  obj_t *obj;

  obj = eval_program("'c' as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(99, obj->intval);

  obj = eval_program("'c' as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(99.0, obj->floatval);

  obj = eval_program("'c' as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("c", obj->stringval);

  obj = eval_program("'c' as boolean")->obj;
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, obj->type);
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_boolean(void) {
  obj_t *obj;

  obj = eval_program("true as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(1, obj->intval);
  obj = eval_program("false as int")->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(0, obj->intval);

  obj = eval_program("true as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(1.0, obj->floatval);
  obj = eval_program("false as float")->obj;
  TEST_ASSERT_EQUAL(TYPE_FLOAT, obj->type);
  TEST_ASSERT_EQUAL(0.0, obj->floatval);

  obj = eval_program("true as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("true", obj->stringval);
  obj = eval_program("false as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("false", obj->stringval);

  obj = eval_program("true as char")->obj;
  TEST_ASSERT_EQUAL(TYPE_CHAR, obj->type);
  TEST_ASSERT_EQUAL('t', obj->intval);
  obj = eval_program("false as char")->obj;
  TEST_ASSERT_EQUAL(TYPE_CHAR, obj->type);
  TEST_ASSERT_EQUAL('f', obj->intval);
}

void test_eval_callable_abs(void) {
  char *program = "abs(-42)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_string_length(void) {
  char *program = "\"Ethel\".length()";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_var_length(void) {
  char *program = "{ val s = \"Ethel\"\n s.length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_length_in_expr(void) {
  char *program = "\"Splunge\".length() * 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(35, obj->intval);
}

void test_eval(void) {
  RUN_TEST(test_eval_calculator);
  RUN_TEST(test_eval_assign_immutable);
  RUN_TEST(test_eval_assign_var);
  RUN_TEST(test_eval_assign_multiple);
  RUN_TEST(test_eval_if_else);
  RUN_TEST(test_eval_if_else_nil);
  RUN_TEST(test_eval_if_else_assign);
  RUN_TEST(test_eval_if_else_assign_expr);
  RUN_TEST(test_eval_boolean_true);
  RUN_TEST(test_eval_boolean_false);
  RUN_TEST(test_eval_truthiness);
  RUN_TEST(test_eval_int_mod);
  RUN_TEST(test_eval_float_mod);
  RUN_TEST(test_eval_numeric_comparison);
  RUN_TEST(test_eval_char_comparison);
  RUN_TEST(test_eval_cast_int);
  RUN_TEST(test_eval_cast_float);
  RUN_TEST(test_eval_cast_string);
  RUN_TEST(test_eval_cast_char);
  RUN_TEST(test_eval_cast_boolean);
  RUN_TEST(test_eval_callable_abs);
  RUN_TEST(test_eval_string_length);
  RUN_TEST(test_eval_string_var_length);
  RUN_TEST(test_eval_string_length_in_expr);
}


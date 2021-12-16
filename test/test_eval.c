#include <stdio.h>
#include "util.h"
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/env.h"
#include "../inc/eval.h"
#include "../inc/rand.h"

obj_t *evaluate(const char* program) {
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  return result->obj;
}

error_t check_error(const char* program) {
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  return result->err;
}

void test_eval_calculator(void) {
  char *program = "4 + 3 * (3+2) + -10/2";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(14, obj->intval);
}

void test_eval_preced_not_astonishing(void) {
  // Different from C++.
  // Inspired by Expert C Programming, p. 45.
  char *program = "5 & 7 == 5 and 1 << 4 + 2 == 18 and ~0 == -1";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(True, obj->boolval);
}

void test_eval_preced_cast(void) {
  char *program = "{ val l = list {\"2\", \"3\", \"4\" } \n"
                  "  val n = l.head() as int             \n"
                  "  n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_add(void) {
  TEST_ASSERT_EQUAL(4, evaluate("2 + 2")->intval);
  TEST_ASSERT_EQUAL(4.1, evaluate("2 + 2.1")->floatval);
  TEST_ASSERT_EQUAL(99, evaluate("2 + 'a'")->intval);
  TEST_ASSERT_EQUAL(4, evaluate("2 + \"2\"")->intval);

  TEST_ASSERT_EQUAL_STRING("foobar", bytearray_to_c_str(evaluate("\"foo\" + \"bar\"")->bytearray));

  TEST_ASSERT_EQUAL(4.1, evaluate("2.1 + 2")->floatval);
  TEST_ASSERT_EQUAL(4.2, evaluate("2.1 + 2.1")->floatval);
  TEST_ASSERT_EQUAL(99.1, evaluate("2.1 + 'a'")->floatval);
  TEST_ASSERT_EQUAL(4.1, evaluate("2.1 + \"2\"")->floatval);

  TEST_ASSERT_EQUAL(99, evaluate("'a' + 2")->byteval);
  TEST_ASSERT_EQUAL(194, evaluate("'a' + 'a'")->byteval);
}

void test_eval_sub(void) {
  TEST_ASSERT_EQUAL(0, evaluate("2 - 2")->intval);
  TEST_ASSERT_EQUAL(-0.1, evaluate("2 - 2.1")->floatval);
  TEST_ASSERT_EQUAL(2, evaluate("99 - 'a'")->intval);

  TEST_ASSERT_EQUAL(0.1, evaluate("2.1 - 2")->floatval);
  TEST_ASSERT_EQUAL(0.1, evaluate("2.2 - 2.1")->floatval);
  TEST_ASSERT_EQUAL(2.1, evaluate("99.1 - 'a'")->floatval);

  TEST_ASSERT_EQUAL(120, evaluate("'z' - 2")->byteval);
  TEST_ASSERT_EQUAL(25, evaluate("'z' - 'a'")->byteval);
}

void test_eval_mul(void) {
  TEST_ASSERT_EQUAL(4, evaluate("2 * 2")->intval);
  TEST_ASSERT_EQUAL(4.2, evaluate("2 * 2.1")->floatval);
  TEST_ASSERT_EQUAL(194, evaluate("2 * 'a'")->intval);

  TEST_ASSERT_EQUAL(4.2, evaluate("2.1 * 2")->floatval);
  TEST_ASSERT_EQUAL(4.4, evaluate("2.1 * 2.1")->floatval);
  TEST_ASSERT_EQUAL(203.7, evaluate("2.1 * 'a'")->floatval);

  TEST_ASSERT_EQUAL(194, evaluate("'a' * 2")->byteval);
  TEST_ASSERT_EQUAL(194, evaluate("'a' * 0x02")->byteval);
}

void test_eval_div(void) {
  TEST_ASSERT_EQUAL(1, evaluate("2 / 2")->intval);
  TEST_ASSERT_EQUAL(4.0, evaluate("10 / 2.5")->floatval);
  TEST_ASSERT_EQUAL(10, evaluate("970 / 'a'")->intval);

  TEST_ASSERT_EQUAL(3.3, evaluate("6.6 / 2")->floatval);
  TEST_ASSERT_EQUAL(3.0, evaluate("6.6 / 2.2")->floatval);
  TEST_ASSERT_EQUAL(5.1, evaluate("500.5 / 'd'")->floatval);

  TEST_ASSERT_EQUAL('2', evaluate("'d' / 2")->byteval);
  TEST_ASSERT_EQUAL(1, evaluate("'a' / 'a'")->byteval);
}

void test_eval_mod(void) {
  TEST_ASSERT_EQUAL(1, evaluate("5 % 2")->intval);
  TEST_ASSERT_EQUAL(96, evaluate("500 % 'e'")->intval);

  TEST_ASSERT_EQUAL(1, evaluate("'a' % 2")->byteval);
  TEST_ASSERT_EQUAL(25, evaluate("'z' % 'a'")->byteval);
}

void test_eval_unary_minus(void) {
  char *program = "2*-1+2*-1";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(-4, obj->intval);
}

void test_eval_assign_immutable(void) {
  char *program = "{\nval x = 32\nx = 33\n}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, result->err);
}

void test_eval_assign_var(void) {
  char *program = "{\nvar x = 32\nx = 33\n}\n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
}

void test_eval_assign_multiple(void) {
  char *program = "{\nvar x = 42\nval y = 9\nx = y\nwhile (x < 12) {\nx = x + 1\n}\nx }\n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, ((obj_t *)result->obj)->intval);
}

void test_eval_del(void) {
  char *program = "{ val x = 42 \n del(x) \n x}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_UNDEFINED, result->err);
}

void test_eval_for_loop_range(void) {
  char *program = "for i in 1..10 { i }";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(10, obj->intval);
}

void test_eval_for_loop_range_step(void) {
  char *program = "{ var n = 0                  \n"
                  "  for i in 1..10 step 4 {    \n"
                  "    ;; 1, 5, 9               \n"
                  "    n = n + i                \n"
                  "  }                          \n"
                  "  n                          \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(15, obj->intval);
}

void test_eval_for_loop_list(void) {
  char *program = "{ val l = list { 1, 3, 5 }   \n"
                  "  var n = 0                  \n"
                  "  for e in l { n = n + e }   \n"
                  "  n                          \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(9, obj->intval);
}

void test_eval_for_loop_dict(void) {
  char *program = "{ val d = dict { 'a': 1, 2: 3 } \n"
                  "  var n = 0                     \n"
                  "  for k in d { n = n + d[k] }   \n"
                  "  n                             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(4, obj->intval);
}

void test_eval_for_loop_arr(void) {
  char *program = "{ val a = arr(3)                \n"
                  "  a[0] = 0x10                   \n"
                  "  a[1] = 0x12                   \n"
                  "  a[2] = 0x14                   \n"
                  "  var n = 0                     \n"
                  "  for i in a { n = n + i }      \n"
                  "  n                             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(54, obj->intval);
}

void test_eval_for_loop_str(void) {
  char *program = "{ val s = \"abc\"               \n"
                  "  var n = 0                     \n"
                  "  for c in s { n = n + c }      \n"
                  "  n                             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(97 + 98 + 99, obj->intval);
}

void test_eval_do_while_loop(void) {
  char *program = "{ var x = 10                \n"
                  "  do {                      \n"
                  "    x = x - 1               \n"
                  "  } while (x > 0)           \n"
                  "  x                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_while_loop(void) {
  char *program = "{ var x = 10                \n"
                  "  while x > 0 { x = x - 1 } \n"
                  "  x                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_for_loop_break(void) {
  char *program = "{ val s = \"abc\"               \n"
                  "  var n = 0                     \n"
                  "  for c in s {                  \n"
                  "    n = n + c                   \n"
                  "    if n > 100 then break       \n"
                  "  }                             \n"
                  "  n                             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(97 + 98, obj->intval);
}

void test_eval_while_loop_break(void) {
  char *program = "{ var x = 10                \n"
                  "  while x > 0 {             \n"
                  "    if x == 3 then break    \n"
                  "    x = x - 1               \n"
                  "  }                         \n"
                  "  x                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_do_while_loop_break(void) {
  char *program = "{ var x = 10                \n"
                  "  do {                      \n"
                  "    x = x - 1               \n"
                  "    if x == 3 then break    \n"
                  "  } while (x > 0)           \n"
                  "  x                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_for_loop_continue(void) {
  char *program = "{ val s = \"abc\"               \n"
                  "  var n = 0                     \n"
                  "  for c in s {                  \n"
                  "    if c == 'b' then continue   \n"
                  "    n = n + c                   \n"
                  "  }                             \n"
                  "  n                             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(97 + 99, obj->intval);
}

void test_eval_while_loop_continue(void) {
  char *program = "{ var x = 10                \n"
                  "  var n = 0                 \n"
                  "  while x > 0 {             \n"
                  "    x = x - 1               \n"
                  "    if x % 2 then continue  \n"
                  "    n = n + 1               \n"
                  "  }                         \n"
                  "  n                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_do_while_loop_continue(void) {
  char *program = "{ var x = 10                \n"
                  "  var n = 0                 \n"
                  "  do {                      \n"
                  "    x = x - 1               \n"
                  "    if x % 2 then continue  \n"
                  "    n = n + 1               \n"
                  "  } while (x > 0)           \n"
                  "  n                         \n"
                  "}                           \n";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else(void) {
  char *program = "if 12 then val x = 5";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_nil(void) {
  char *program = "if 0 then 5";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(obj));
}

void test_eval_if_else_assign(void) {
  char *program = "if 12 then val x = 5";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_assign_expr(void) {
  char *program = "val x = if 12 then 5";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_boolean_true(void) {
  char *program = "val x = 1 and (1 or 0) or (0 and 1 or 1)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_boolean_false(void) {
  char *program = "val x = 0 and (1 or 0) or (0 and 1 or 0)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 0 is boolean false.
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_logical_not(void) {
  char *program = "not false == not(false == true)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, result->obj->intval);
}

void test_eval_truthiness(void) {
  char *program = "val x = (\"glug\" or 0) and (4.3 or 0) and (0 or 'c')";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_numeric_comparison(void) {
  char *program = "if 5 < 3.1 then val x = 2 else val x = if 5>= 3 then 42 else -1";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_char_comparison(void) {
  char *program = "if 'c' > 'd' then 'e' else if 'f' == 'f' then 'g'";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL('g', obj->byteval);
}

void test_eval_cast_int(void) {
  obj_t *obj;

  obj = evaluate("-99 as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-99, obj->intval);

  obj = evaluate("-99 as float");
  TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-99.0, obj->floatval);

  obj = evaluate("-99 as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("-99", bytearray_to_c_str(obj->bytearray));

  obj = evaluate("99 as byte");
  TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(obj));
  TEST_ASSERT_EQUAL('c', obj->byteval);

  obj = evaluate("-99 as boolean");
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(obj));
  TEST_ASSERT_EQUAL(1, obj->boolval);
}

void test_eval_cast_float(void) {
  obj_t *obj;

  obj = evaluate("-42.99 as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-42, obj->intval);

  obj = evaluate("-42.99 as float");
  TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-42.99, obj->floatval);

  obj = evaluate("-42.02 as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("-42.020000", bytearray_to_c_str(obj->bytearray));

  obj = evaluate("-42.99 as boolean");
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(obj));
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_string(void) {
  obj_t *obj;

  obj = evaluate("\"-12.44\" as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-12, obj->intval);

  obj = evaluate("\"-42.99\" as float");
  TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(-42.99, obj->floatval);

  obj = evaluate("\"-42.1\" as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("-42.1", bytearray_to_c_str(obj->bytearray));

  obj = evaluate("\"-42.99\" as boolean");
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(obj));
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_char(void) {
  obj_t *obj;

  obj = evaluate("'c' as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(99, obj->intval);

  obj = evaluate("'c' as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("c", bytearray_to_c_str(obj->bytearray));

  obj = evaluate("'t' as boolean");
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(obj));
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_cast_boolean(void) {
  obj_t *obj;

  obj = evaluate("true as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(1, obj->intval);
  obj = evaluate("false as int");
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(obj));
  TEST_ASSERT_EQUAL(0, obj->intval);

  obj = evaluate("true as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("true", bytearray_to_c_str(obj->bytearray));
  obj = evaluate("false as string");
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("false", bytearray_to_c_str(obj->bytearray));

  obj = evaluate("true as byte");
  TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(obj));
  TEST_ASSERT_EQUAL('t', obj->byteval);
  obj = evaluate("false as byte");
  TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(obj));
  TEST_ASSERT_EQUAL('f', obj->byteval);
}

void test_eval_is_type(void) {
  TEST_ASSERT_EQUAL(True, (evaluate("2 is int"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("2 is float"))->boolval);

  TEST_ASSERT_EQUAL(True, (evaluate("2.2 is float"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("2.2 is boolean"))->boolval);

  TEST_ASSERT_EQUAL(True, (evaluate("true is boolean"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("true is string"))->boolval);

  TEST_ASSERT_EQUAL(True, (evaluate("\"glug\" is string"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("\"glug\" is byte"))->boolval);

  TEST_ASSERT_EQUAL(True, (evaluate("'c' is byte"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("'c' is int"))->boolval);

  TEST_ASSERT_EQUAL(True, (evaluate("\"moo\" is string"))->boolval);
  TEST_ASSERT_EQUAL(False, (evaluate("\"moo\" is int"))->boolval);

  // TODO There's currently no way to do this for bytearray.
  // Want a proper list of types that includes keyword etc, so we can say type(while) etc.
  // Possible hook for documentation, as well.
}

void test_eval_type_of(void) {
  TEST_ASSERT_EQUAL_STRING("Int",        bytearray_to_c_str(evaluate("type(2)")->bytearray));
  TEST_ASSERT_EQUAL_STRING("Float",      bytearray_to_c_str(evaluate("type(2.2)")->bytearray));
  TEST_ASSERT_EQUAL_STRING("Bool",       bytearray_to_c_str(evaluate("type(true)")->bytearray));
  TEST_ASSERT_EQUAL_STRING("Str",        bytearray_to_c_str(evaluate("type(\"2\")")->bytearray));
  TEST_ASSERT_EQUAL_STRING("Byte Array", bytearray_to_c_str(evaluate("type(arr(2))")->bytearray));
}

void test_eval_callable_abs(void) {
  char *program = "abs(-42)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_callable_rand(void) {
  char *program = "rand(1)";  // A number between 0 and 0.
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(0, result->obj->intval);

  rand32_init();  // Reset RNG to default initial seed.
  program = "rand(100)";
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(79, result->obj->intval);
}

void test_eval_callable_hex(void) {
  char *program = "hex(-199)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("0xffffff39", bytearray_to_c_str(obj->bytearray));
}

void test_eval_callable_bin(void) {
  char *program = "bin(177)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(obj));
  TEST_ASSERT_EQUAL_STRING("0b10110001", bytearray_to_c_str(obj->bytearray));
}

void test_eval_string_length(void) {
  char *program = "\"Ethel\".length()";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_var_length(void) {
  char *program = "{ val s = \"Ethel\"\n s.length() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_length_in_expr(void) {
  char *program = "\"Splunge\".length() * 5";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(35, obj->intval);
}

void test_eval_list_val_length(void) {
  char *program = "{ val l = list { 1, 2, 3}\n"
                  "  l.length() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_list_val_eq(void) {
  char *program = "{ val a = list { 1, 2, 3 }   \n"
                  "  val b = list { 0, 1, 2, 3 }\n"
                  "  var i = 0                         \n"
                  "  if (a == b.tail()) then i = 1     \n"
                  "  if (a == b) then i = 0            \n"
                  "  i                                 \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_get(void) {
  char *program = "{ val l = list { 1, 2.3, 'c' }\n"
                  "  l.get(0) }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  char *program1= "{ val l = list { 1, 2.3, 'c' }\n"
                  "  l.get(1) }";
  eval_program(program1, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2.3, result->obj->floatval);

  char *program2= "{ val l = list { 1, 2.3, 'c' }\n"
                  "  l.get(2) }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL('c', result->obj->byteval);

  char *program3 = "{ val l = list { 1, 2.3, 'c' }\n"
                   "  l.get(-1) }";
  eval_program(program3, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL('c', result->obj->byteval);

  char *program4= "{ val l = list { 1, 2.3, 'c' }\n"
                  "  l.get(-2) }";
  eval_program(program4, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2.3, result->obj->floatval);

  char *program5= "{ val l = list { 1, 2.3, 'c' }\n"
                  "  l.get(-3) }";
  eval_program(program5, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list \n"
                   "  l.get(0) }";
  eval_program(program6, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program7 = "{ val l = list \n"
                   "  l.get(-1) }";
  eval_program(program7, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program8 = "{ val l = list { 1, 2.3, 'c' }\n"
                   "  l.get(14) }";
  eval_program(program8, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program9 = "{ val l = list { 1, 2.3, 'c' }\n"
                   "  l.get(-5) }";
  eval_program(program9, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));
}

void test_eval_list_val_head(void) {
  char *program = "{ val l = list { 1, 2, 3}\n"
                  "  l.head() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_tail_length(void) {
  char *program = "{ val l = list { 1, 2, 3}\n"
                  "  l.tail().length() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head(void) {
  char *program = "{ val l = list { 1, 2, 3, 4, 5}\n"
                  "  l.slice(1, 3).head() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head_tail_length(void) {
  char *program = "{ val l = list { 1, 2, 3, 4, 5}\n"
                  "  l.slice(1, 3).tail().length() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_prepend(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.prepend(6)\n"
                  "  l.head() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list \n"
                   "  l.prepend(8)\n"
                   "  l.head() }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(8, result->obj->intval);
}

void test_eval_list_val_append(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.append(6)\n"
                  "  l.get(3) }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list \n"
                   "  l.append(8)\n"
                   "  l.head() }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(8, result->obj->intval);
}

void test_eval_list_val_remove_first(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeFirst() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list \n"
                   "  l.removeFirst() }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));
}

void test_eval_list_val_remove_last(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeLast() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(3, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list \n"
                   "  l.removeLast() }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));
}

void test_eval_list_val_remove_at(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeAt(0)\n"
                  "  l.get(0) }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  char *program1= "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeAt(1)\n"
                  "  l.get(1) }";
  eval_program(program1, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(3, result->obj->intval);

  char *program2= "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeAt(2)\n"
                  "  l.get(1) }";
  eval_program(program2, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  char *program3 = "{ val l = list { 1, 2, 3 }\n"
                   "  l.removeAt(-1)\n"
                   "  l.get(-1) }";
  eval_program(program3, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  char *program4= "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeAt(-2)\n"
                  "  l.get(1) }";
  eval_program(program4, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(3, result->obj->intval);

  char *program5= "{ val l = list { 1, 2, 3 }\n"
                  "  l.removeAt(-3)\n"
                  "  l.get(0) }";
  eval_program(program5, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list \n"
                   "  l.removeAt(0) }";
  eval_program(program6, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program7 = "{ val l = list \n"
                   "  l.removeAt(-1) }";
  eval_program(program7, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program8 = "{ val l = list { 1, 2, 3 }\n"
                   "  l.removeAt(14) }";
  eval_program(program8, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));

  char *program9 = "{ val l = list { 1, 2, 3 }\n"
                   "  l.removeAt(-5) }";
  eval_program(program9, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));
}

void test_eval_list_in(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  2 in l }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_arr_decl(void) {
  char *program = "{ val a = arr(12)\n"
                  "  a.length() }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);

  char *program2= "{ val a = arr(12)\n"
                  "  a[3] }";
  eval_program(program2, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(result->obj));
  TEST_ASSERT_EQUAL('\0', result->obj->byteval);
}

void test_eval_arr_assign(void) {
  // Assign with int.
  char *program = "{ val a = arr(12)\n"
                  "  a[4] = 42\n"
                  "  a[4] }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL('*', result->obj->byteval);

  // Assign with char.
  char *program2= "{ val a = arr(12)\n"
                  "  a[3] = 'x'\n"
                  "  a[3] }";
  eval_program(program2, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(result->obj));
  TEST_ASSERT_EQUAL('x', result->obj->byteval);
}

void test_eval_bitwise_or(void) {
  char *program = "4 | 0x8";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);
}

void test_eval_bitwise_xor(void) {
  char *program = "15 ^ 0b0111";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(8, result->obj->intval);
}

void test_eval_bitwise_and(void) {
  char *program = "15 & 0x07";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(7, result->obj->intval);
}

void test_eval_bitwise_not(void) {
  char *program = "~0";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(-1, result->obj->intval);
}

void test_eval_bitwise_shl(void) {
  char *program = "0x1 << 0x4";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(16, result->obj->intval);
}

void test_eval_bitwise_shr(void) {
  char *program = "64 >> 4";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(4, result->obj->intval);
}

void test_eval_function(void) {
  char *program = "{ val f = fn(x) { x + 1 } \n"
                  "  f(1) }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(result->obj));
  TEST_ASSERT_EQUAL(2, result->obj->intval);
}

void test_eval_function_undef(void) {
  char *program = "nope(42)";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_FUNCTION_UNDEFINED, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(result->obj));
}

void test_eval_function_wrong_args(void) {
  char *program = "{ val f = fn(x, y) { x + y } \n"
                  "  f(1) }";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_WRONG_ARG_COUNT, result->err);
}

void test_eval_function_return(void) {
  char *program = "{ val f = fn(x) {\n"
                  "    if x == 0 then return 42\n"
                  "    x\n"
                  "  } \n"
                  "  f(0)}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_function_lexical_scope(void) {
  char *program = "{ val x = 42            \n"
                  "  val f = fn()  {   x } \n"
                  "  val g = fn(x) { f() } \n"
                  "  g(17)                 \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_function_composition(void) {
  char *program = "{ val f = fn(x) { x + 1 } \n"
                  "  val g = fn(x) { x + 2 } \n"
                  "  f(g(f(g(1))))           \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(7, result->obj->intval);
}

void test_eval_func_in_list(void) {
  char *program = "{ val l = list            \n"
                  "  val f = fn(x) { x + 1 } \n"
                  "  l.append(f)             \n"
                  "  l[f(-1)](5)             \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);
}

void test_eval_func_in_dict(void) {
  char *program = "{ val d = dict                            \n"
                  "  val f = fn(x) { x + 1 }                 \n"
                  "  var i = 0                               \n"
                  "  d[\"incr\"] = f                         \n"
                  "  d[\"decr\"] = fn(x) { x - 1 }           \n"
                  "  if (d[\"incr\"](0) == 1) then i = i + 1 \n"
                  "  if (d[\"decr\"](1) == 0) then i = i + 1 \n"
                  "  i                                       \n"
                  "}";

  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

}

void test_eval_block_scope(void) {
  char *program = "{ val x = 42       \n"
                  "  { { { x } } }    \n"
                  "}                  \n";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_in_list(void) {
  char *program = "{ val l = list { 1, 2, 3 }\n"
                  "  5 in l}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val l = list { 1, 2, 3 }\n"
            "  2 in l}";
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_range(void) {
  char *program = "{ val s = \"glug\"\n"
                  "  0 in 1..s.length()}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val s = \"glug\"\n"
            "  3 in 1..s.length()}";
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_string(void) {
  char *program = "{ val s = \"Ethel\"\n"
                  "  'x' in s}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val s = \"Ethel\"\n"
            "  'e' in s}";
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_bytearray(void) {
  char *program = "{ val a = arr(10)\n"
                  "  1 in a}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val a = arr(10)\n"
            "  a[4] = 1 \n"
            "  1 in a}";
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_arr_subscript_cmp(void) {
  char *program = "{ val s = \"foo\" \n"
                  "  var i = 0 \n"
                  "  if (s[0] == 'f') then i = i + 1  \n"
                  "  if (s[0] <= 'f') then i = i + 1  \n"
                  "  if (s[0] >= 'f') then i = i + 1  \n"
                  "  if (s[0] != 'g') then i = i + 1  \n"
                  "  if (s[0] < 'g')  then i = i + 1  \n"
                  "  if (s[0] > 'e')  then i = i + 1  \n"
                  "  if ('f' == s[0]) then i = i + 1  \n"
                  "  if ('f' <= s[0]) then i = i + 1  \n"
                  "  if ('f' >= s[0]) then i = i + 1  \n"
                  "  if ('e' != s[0]) then i = i + 1  \n"
                  "  if ('e' < s[0])  then i = i + 1  \n"
                  "  if ('g' > s[0])  then i = i + 1  \n"
                  "  i                                \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);
}

void test_eval_arr_subscript_assign_byte(void) {
  char *program = "{ val swap = fn(a, i, j) {         \n"
                  "     val temp = a[i]               \n"
                  "     a[i] = a[j]                   \n"
                  "     a[j] = temp                   \n"
                  "  }                                \n"
                  "  val s = \"I like potatoes\"      \n"
                  "  swap(s, 0, 3)                    \n"
                  "  s                                \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL_STRING("i lIke potatoes", bytearray_to_c_str(result->obj->bytearray));
}

void test_eval_arr_subscript_assign_int(void) {
  char *program = "{ val s = \"I like potatoes\"      \n"
                  "  s[0] = 85                        \n"
                  "  s[2] = 0x4c                      \n"
                  "  s                                \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL_STRING("U Like potatoes", bytearray_to_c_str(result->obj->bytearray));
}

void test_eval_lhs_not_assignable(void) {
  TEST_ASSERT_EQUAL(ERR_LHS_NOT_ASSIGNABLE, check_error("2 = 3"));
  TEST_ASSERT_EQUAL(ERR_LHS_NOT_ASSIGNABLE, check_error("2.1 = 2.2"));
  TEST_ASSERT_EQUAL(ERR_LHS_NOT_ASSIGNABLE, check_error("'c' = 'd'"));
  TEST_ASSERT_EQUAL(ERR_LHS_NOT_ASSIGNABLE, check_error("\"foo\" = \"bar\""));
  TEST_ASSERT_EQUAL(ERR_LHS_NOT_ASSIGNABLE, check_error("arr(2) = 4"));
}

void test_eval_str_compare(void) {
  char *program = "{ val s1 = \"foo\"             \n"
                  "  val s2 = \"bar\"             \n"
                  "  var i = 0                    \n"
                  "  if (s1 == s1) then i = i + 1 \n"
                  "  if (s1 != s1) then i = i - 1 \n"
                  "  if (s1 == s2) then i = i - 1 \n"
                  "  if (s1 != s2) then i = i + 1 \n"
                  "  i                            \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);
}

void test_eval_dict(void) {
  char *program = "{ val d = dict    \n"
                  "  var i = 0       \n"
                  "  d[2] = 10       \n"
                  "  d[\"foo\"] = 1  \n"
                  "  d[3.3] = 1      \n"
                  "  d['c'] = 1      \n"
                  "  if (1 in d) then       i = i - 1 \n"
                  "  if (\"no\" in d) then  i = i - 1 \n"
                  "  if (3.4 in d) then     i = i - 1 \n"
                  "  if ('x' in d) then     i = i - 1 \n"
                  "  if (2 in d) then       i = i + 1 \n"
                  "  if (\"foo\" in d) then i = i + 1 \n"
                  "  if (3.3 in d) then     i = i + 1 \n"
                  "  if ('c' in d) then     i = i + 1 \n"
                  "  i = i + d[2]                     \n"
                  "  i \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(14, result->obj->intval);
}

void test_eval_dict_len(void) {
  char *program = "{ val d = dict                      \n"
                  "  var i = d.length()                \n"
                  "  d['a'] = 1                        \n"
                  "  ;; 'a' and 97 hash collide        \n"
                  "  d[97] = 1                         \n"
                  "  d[0] = 1                          \n"
                  "  d[1] = 1                          \n"
                  "  i = i + d.length()                \n"
                  "  i                                 \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(4, result->obj->intval);
}

void test_eval_dict_remove(void) {
  char *program = "{ val d = dict                      \n"
                  "  d['a'] = 1                        \n"
                  "  ;; 'a' and 97 hash collide        \n"
                  "  d[97] = 2                         \n"
                  "  d.remove('a')                     \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);
}

void test_eval_dict_keys(void) {
  char *program = "{ val d = dict                        \n"
                  "  d['a'] = 1                          \n"
                  "  d[97] = 2                           \n"
                  "  d[true] = 3                         \n"
                  "  d[1] = 4                            \n"
                  "  val l = d.keys()                    \n"
                  "  var i = 0                           \n"
                  "  if ('a' in l)  then i = i + 1       \n"
                  "  if (97 in l)   then i = i + 1       \n"
                  "  if (true in l) then i = i + 1       \n"
                  "  if (1 in l)    then i = i + 1       \n"
                  "  if (l.length() == 4) then i = i + 1 \n"
                  "  i                                   \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(5, result->obj->intval);
}

void test_eval_iterable_random_choice() {
  rand32_init();
  char *program = "{ var i = 0;                             \n"
                  "  val l = list {'a', 'b', 'c'}           \n"
                  "  val d = dict { 1: 1, 2: 1, 3: 1}       \n"
                  "  val r = 1..10                          \n"
                  "  val s = \"I like pie\"                 \n"
                  "  if l.rand() == 'c'      then i = i + 1 \n"
                  "  if d.keys().rand() == 3 then i = i + 1 \n"
                  "  if r.rand() == 6        then i = i + 1 \n"
                  "  if s.rand() == 'p'      then i = i + 1 \n"
                  "  i                                      \n"
                  "}";
  eval_result_t* result = mem_alloc(sizeof(eval_result_t));
  eval_program(program, result);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(4, result->obj->intval);
}

void test_eval(void) {
  RUN_TEST(test_eval_preced_not_astonishing);
  RUN_TEST(test_eval_preced_cast);
  RUN_TEST(test_eval_add);
  RUN_TEST(test_eval_sub);
  RUN_TEST(test_eval_mul);
  RUN_TEST(test_eval_div);
  RUN_TEST(test_eval_mod);
  RUN_TEST(test_eval_unary_minus);
  RUN_TEST(test_eval_assign_immutable);
  RUN_TEST(test_eval_assign_var);
  RUN_TEST(test_eval_assign_multiple);
  RUN_TEST(test_eval_del);
  RUN_TEST(test_eval_for_loop_range);
  RUN_TEST(test_eval_for_loop_range_step);
  RUN_TEST(test_eval_for_loop_list);
  RUN_TEST(test_eval_for_loop_dict);
  RUN_TEST(test_eval_for_loop_arr);
  RUN_TEST(test_eval_for_loop_str);
  RUN_TEST(test_eval_do_while_loop);
  RUN_TEST(test_eval_while_loop);
  RUN_TEST(test_eval_for_loop_break);
  RUN_TEST(test_eval_while_loop_break);
  RUN_TEST(test_eval_do_while_loop_break);
  RUN_TEST(test_eval_for_loop_continue);
  RUN_TEST(test_eval_while_loop_continue);
  RUN_TEST(test_eval_do_while_loop_continue);
  RUN_TEST(test_eval_if_else);
  RUN_TEST(test_eval_if_else_nil);
  RUN_TEST(test_eval_if_else_assign);
  RUN_TEST(test_eval_if_else_assign_expr);
  RUN_TEST(test_eval_boolean_true);
  RUN_TEST(test_eval_boolean_false);
  RUN_TEST(test_eval_logical_not);
  RUN_TEST(test_eval_truthiness);
  RUN_TEST(test_eval_numeric_comparison);
  RUN_TEST(test_eval_char_comparison);
  RUN_TEST(test_eval_cast_int);
  RUN_TEST(test_eval_cast_float);
  RUN_TEST(test_eval_cast_string);
  RUN_TEST(test_eval_cast_char);
  RUN_TEST(test_eval_cast_boolean);
  RUN_TEST(test_eval_is_type);
  RUN_TEST(test_eval_type_of);
  RUN_TEST(test_eval_callable_abs);
  RUN_TEST(test_eval_callable_rand);
  RUN_TEST(test_eval_callable_hex);
  RUN_TEST(test_eval_callable_bin);
  RUN_TEST(test_eval_string_length);
  RUN_TEST(test_eval_string_var_length);
  RUN_TEST(test_eval_string_length_in_expr);
  RUN_TEST(test_eval_list_val_length);
  RUN_TEST(test_eval_list_val_eq);
  RUN_TEST(test_eval_list_val_get);
  RUN_TEST(test_eval_list_val_head);
  RUN_TEST(test_eval_list_val_tail_length);
  RUN_TEST(test_eval_list_val_slice_head);
  RUN_TEST(test_eval_list_val_slice_head_tail_length);
  RUN_TEST(test_eval_list_val_prepend);
  RUN_TEST(test_eval_list_val_append);
  RUN_TEST(test_eval_list_val_remove_first);
  RUN_TEST(test_eval_list_val_remove_last);
  RUN_TEST(test_eval_list_val_remove_at);
  RUN_TEST(test_eval_list_in);
  RUN_TEST(test_eval_arr_decl);
  RUN_TEST(test_eval_arr_assign);
  RUN_TEST(test_eval_bitwise_or);
  RUN_TEST(test_eval_bitwise_xor);
  RUN_TEST(test_eval_bitwise_and);
  RUN_TEST(test_eval_bitwise_not);
  RUN_TEST(test_eval_bitwise_shl);
  RUN_TEST(test_eval_bitwise_shr);
  RUN_TEST(test_eval_function);
  RUN_TEST(test_eval_function_undef);
  RUN_TEST(test_eval_function_wrong_args);
  RUN_TEST(test_eval_function_return);
  RUN_TEST(test_eval_function_lexical_scope);
  RUN_TEST(test_eval_function_composition);
  RUN_TEST(test_eval_func_in_list);
  RUN_TEST(test_eval_func_in_dict);
  RUN_TEST(test_eval_block_scope);
  RUN_TEST(test_eval_in_list);
  RUN_TEST(test_eval_in_range);
  RUN_TEST(test_eval_in_string);
  RUN_TEST(test_eval_in_bytearray);
  RUN_TEST(test_eval_arr_subscript_cmp);
  RUN_TEST(test_eval_arr_subscript_assign_byte);
  RUN_TEST(test_eval_arr_subscript_assign_int);
  RUN_TEST(test_eval_lhs_not_assignable);
  RUN_TEST(test_eval_str_compare);
  RUN_TEST(test_eval_dict);
  RUN_TEST(test_eval_dict_len);
  RUN_TEST(test_eval_dict_remove);
  RUN_TEST(test_eval_dict_keys);
  RUN_TEST(test_eval_iterable_random_choice);
}


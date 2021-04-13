#include <stdio.h>
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/str.h"
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
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(14, obj->intval);
}

void test_eval_assign_immutable(void) {
  char *program = "{\nval x = 32\nx = 33\n}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, result->err);
}

void test_eval_assign_var(void) {
  char *program = "{\nvar x = 32\nx = 33\n}\n";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
}

void test_eval_assign_multiple(void) {
  char *program = "{\nvar x = 42\nval y = 9\nx = y\nwhile (x < 12) {\nx = x + 1\n}\nx }\n";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, ((obj_t *)result->obj)->intval);
}

void test_eval_if_else(void) {
  char *program = "if 12 then val x = 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_nil(void) {
  char *program = "if 0 then 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_NIL, obj->type);
}

void test_eval_if_else_assign(void) {
  char *program = "if 12 then val x = 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_if_else_assign_expr(void) {
  char *program = "val x = if 12 then 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_boolean_true(void) {
  char *program = "val x = 1 and (1 or 0) or (0 and 1 or 1)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_boolean_false(void) {
  char *program = "val x = 0 and (1 or 0) or (0 and 1 or 0)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 0 is boolean false.
  TEST_ASSERT_EQUAL(0, obj->intval);
}

void test_eval_truthiness(void) {
  char *program = "val x = (\"glug\" or 0) and (4.3 or 0) and (0 or 'c')"; 
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_int_mod(void) {
  char *program = "7 mod 4";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_float_mod(void) {
  char *program = "16.2 mod 3";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  // Modulus is always an int in our world.
  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_numeric_comparison(void) {
  char *program = "if 5 < 3.1 then val x = 2 else val x = if 5>= 3 then 42 else -1";

  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_char_comparison(void) {
  char *program = "if 'c' > 'd' then 'e' else if 'f' == 'f' then 'g'";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

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
  TEST_ASSERT_EQUAL_STRING("-99", bytearray_to_c_str(obj->bytearray));

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
  TEST_ASSERT_EQUAL_STRING("-42.020000", bytearray_to_c_str(obj->bytearray));

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
  TEST_ASSERT_EQUAL_STRING("-42.1", bytearray_to_c_str(obj->bytearray));

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
  TEST_ASSERT_EQUAL_STRING("c", bytearray_to_c_str(obj->bytearray));

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
  TEST_ASSERT_EQUAL_STRING("true", bytearray_to_c_str(obj->bytearray));
  obj = eval_program("false as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("false", bytearray_to_c_str(obj->bytearray));

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
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(42, obj->intval);
}

void test_eval_callable_hex(void) {
  char *program = "hex(-199)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("0xffffff39", bytearray_to_c_str(obj->bytearray));
}

void test_eval_callable_bin(void) {
  char *program = "bin(177)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("0b10110001", bytearray_to_c_str(obj->bytearray));
}

void test_eval_string_length(void) {
  char *program = "\"Ethel\".length()";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_var_length(void) {
  char *program = "{ val s = \"Ethel\"\n s.length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(5, obj->intval);
}

void test_eval_string_length_in_expr(void) {
  char *program = "\"Splunge\".length() * 5";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(35, obj->intval);
}

void test_eval_list_val_length(void) {
  char *program = "{ val l = list of Int { 1, 2, 3}\nl.length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_list_val_get(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.get(0) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  char *program1= "{ val l = list of Int { 1, 2, 3 }\nl.get(1) }";
  eval_result_t *result1 = eval_program(program1);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result1->err);
  TEST_ASSERT_EQUAL(2, result1->obj->intval);

  char *program2= "{ val l = list of Int { 1, 2, 3 }\nl.get(2) }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(3, result2->obj->intval);

  char *program3 = "{ val l = list of Int { 1, 2, 3 }\nl.get(-1) }";
  eval_result_t *result3 = eval_program(program3);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result3->err);
  TEST_ASSERT_EQUAL(3, result3->obj->intval);

  char *program4= "{ val l = list of Int { 1, 2, 3 }\nl.get(-2) }";
  eval_result_t *result4 = eval_program(program4);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result4->err);
  TEST_ASSERT_EQUAL(2, result4->obj->intval);

  char *program5= "{ val l = list of Int { 1, 2, 3 }\nl.get(-3) }";
  eval_result_t *result5 = eval_program(program5);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result5->err);
  TEST_ASSERT_EQUAL(1, result5->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list of Int\nl.get(0) }";
  eval_result_t *result6 = eval_program(program6);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result6->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result6->obj->type);

  char *program7 = "{ val l = list of Int\nl.get(-1) }";
  eval_result_t *result7 = eval_program(program7);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result7->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result7->obj->type);

  char *program8 = "{ val l = list of Int { 1, 2, 3 }\nl.get(14) }";
  eval_result_t *result8 = eval_program(program8);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result8->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result8->obj->type);

  char *program9 = "{ val l = list of Int { 1, 2, 3 }\nl.get(-5) }";
  eval_result_t *result9 = eval_program(program9);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result9->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result9->obj->type);
}

void test_eval_list_val_head(void) {
  char *program = "{ val l = list of Int { 1, 2, 3}\nl.head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_tail_length(void) {
  char *program = "{ val l = list of Int { 1, 2, 3}\nl.tail().length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head(void) {
  char *program = "{ val l = list of Int { 1, 2, 3, 4, 5}\nl.slice(1, 3).head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head_tail_length(void) {
  char *program = "{ val l = list of Int { 1, 2, 3, 4, 5}\nl.slice(1, 3).tail().length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_prepend(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.prepend(6)\nl.head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list of Int\nl.prepend(8)\nl.head() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(8, result2->obj->intval);
}

void test_eval_list_val_append(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.append(6)\nl.get(3) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list of Int\nl.append(8)\nl.head() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(8, result2->obj->intval);
}

void test_eval_list_val_remove_first(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.removeFirst() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list of Int\nl.removeFirst() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result2->obj->type);
}

void test_eval_list_val_remove_last(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.removeLast() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(3, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list of Int\nl.removeLast() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result2->obj->type);
}

void test_eval_list_val_remove_at(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(0)\nl.get(0) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  char *program1= "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(1)\nl.get(1) }";
  eval_result_t *result1 = eval_program(program1);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result1->err);
  TEST_ASSERT_EQUAL(3, result1->obj->intval);

  char *program2= "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(2)\nl.get(1) }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(2, result2->obj->intval);

  char *program3 = "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(-1)\nl.get(-1) }";
  eval_result_t *result3 = eval_program(program3);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result3->err);
  TEST_ASSERT_EQUAL(2, result3->obj->intval);

  char *program4= "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(-2)\nl.get(1) }";
  eval_result_t *result4 = eval_program(program4);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result4->err);
  TEST_ASSERT_EQUAL(3, result4->obj->intval);

  char *program5= "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(-3)\nl.get(0) }";
  eval_result_t *result5 = eval_program(program5);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result5->err);
  TEST_ASSERT_EQUAL(2, result5->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list of Int\nl.removeAt(0) }";
  eval_result_t *result6 = eval_program(program6);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result6->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result6->obj->type);

  char *program7 = "{ val l = list of Int\nl.removeAt(-1) }";
  eval_result_t *result7 = eval_program(program7);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result7->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result7->obj->type);

  char *program8 = "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(14) }";
  eval_result_t *result8 = eval_program(program8);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result8->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result8->obj->type);

  char *program9 = "{ val l = list of Int { 1, 2, 3 }\nl.removeAt(-5) }";
  eval_result_t *result9 = eval_program(program9);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result9->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result9->obj->type);
}

void test_eval_arr_decl(void) {
  char *program = "{ val a = arr(12)\n a.length() }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);

  char *program2= "{ val a = arr(12)\n a[3] }";
  eval_result_t *result2= eval_program(program2);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_CHAR, result2->obj->type);
  TEST_ASSERT_EQUAL('\0', result2->obj->charval);
}

void test_eval_arr_assign(void) {
  // Assign with int.
  char *program = "{ val a = arr(12)\n a[4] = 42\n a[4] }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL('*', result->obj->charval);

  // Assign with char.
  char *program2= "{ val a = arr(12)\n a[3] = 'x'\n a[3] }";
  eval_result_t *result2= eval_program(program2);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_CHAR, result2->obj->type);
  TEST_ASSERT_EQUAL('x', result2->obj->charval);
}

void test_eval_bitwise_or(void) {
  char *program = "4 | 0x8";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);
}

void test_eval_bitwise_xor(void) {
  char *program = "15 ^ 0b0111";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(8, result->obj->intval);
}

void test_eval_bitwise_and(void) {
  char *program = "15 & 0x07";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(7, result->obj->intval);
}

void test_eval_bitwise_not(void) {
  char *program = "~0";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(-1, result->obj->intval);
}

void test_eval_bitwise_shl(void) {
  char *program = "0x1 << 0x4";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(16, result->obj->intval);
}

void test_eval_bitwise_shr(void) {
  char *program = "64 >> 4";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(4, result->obj->intval);
}

void test_eval_function(void) {
  char *program = "{ val f = fn(x) { x + 1 } \nf(1) }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_INT, result->obj->type);
  TEST_ASSERT_EQUAL(2, result->obj->intval);
}

void test_eval_function_wrong_args(void) {
  char *program = "{ val f = fn(x, y) { x + y } \nf(1) }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_WRONG_ARG_COUNT, result->err);
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
  RUN_TEST(test_eval_callable_hex);
  RUN_TEST(test_eval_callable_bin);
  RUN_TEST(test_eval_string_length);
  RUN_TEST(test_eval_string_var_length);
  RUN_TEST(test_eval_string_length_in_expr);
  RUN_TEST(test_eval_list_val_length);
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
  RUN_TEST(test_eval_arr_decl);
  RUN_TEST(test_eval_arr_assign);
  RUN_TEST(test_eval_bitwise_or);
  RUN_TEST(test_eval_bitwise_xor);
  RUN_TEST(test_eval_bitwise_and);
  RUN_TEST(test_eval_bitwise_not);
  RUN_TEST(test_eval_bitwise_shl);
  RUN_TEST(test_eval_bitwise_shr);
  RUN_TEST(test_eval_function);
  RUN_TEST(test_eval_function_wrong_args);
}


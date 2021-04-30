#include <stdio.h>
#include "unity/unity.h"
#include "test_eval.h"
#include "../inc/str.h"
#include "../inc/env.h"
#include "../inc/eval.h"

eval_result_t *eval_program(char* program) {
  env_t env;
  env_init(&env);

  enter_scope(&env);

  return eval(&env, program);
  
  leave_scope(&env);
}

void test_eval_calculator(void) {
  char *program = "4 + 3 * (3+2) + -10/2";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(14, obj->intval);
}

void test_eval_unary_minus(void) {
  char *program = "2*-1+2*-1";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(-4, obj->intval);
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

void test_eval_del(void) {
  char *program = "{ val x = 42 \n del(x) \n x}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_UNDEFINED, result->err);
}

void test_eval_for_loop(void) {
  char *program = "for i in 1..10 { i }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(10, obj->intval);
}

void test_eval_while_loop(void) {
  char *program = "{ var x = 10                \n"
                  "  while x > 0 { x = x - 1 } \n"
                  "  x                         \n"
                  "}                           \n";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(0, obj->intval);
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

void test_eval_logical_not(void) {
  char *program = "not false == not(false == true)";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  // 1 is boolean true.
  TEST_ASSERT_EQUAL(1, result->obj->intval);
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
  TEST_ASSERT_EQUAL('g', obj->byteval);
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

  obj = eval_program("99 as byte")->obj;
  TEST_ASSERT_EQUAL(TYPE_BYTE, obj->type);
  TEST_ASSERT_EQUAL('c', obj->byteval);

  obj = eval_program("-99 as boolean")->obj;
  TEST_ASSERT_EQUAL(TYPE_BOOLEAN, obj->type);
  TEST_ASSERT_EQUAL(1, obj->boolval);
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

  obj = eval_program("'c' as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("c", bytearray_to_c_str(obj->bytearray));

  obj = eval_program("'t' as boolean")->obj;
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

  obj = eval_program("true as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("true", bytearray_to_c_str(obj->bytearray));
  obj = eval_program("false as string")->obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL_STRING("false", bytearray_to_c_str(obj->bytearray));

  obj = eval_program("true as byte")->obj;
  TEST_ASSERT_EQUAL(TYPE_BYTE, obj->type);
  TEST_ASSERT_EQUAL('t', obj->byteval);
  obj = eval_program("false as byte")->obj;
  TEST_ASSERT_EQUAL(TYPE_BYTE, obj->type);
  TEST_ASSERT_EQUAL('f', obj->byteval);
}

void test_eval_is_type(void) {
  TEST_ASSERT_EQUAL(True, (eval_program("2 is int")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("2 is float")->obj)->boolval);

  TEST_ASSERT_EQUAL(True, (eval_program("2.2 is float")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("2.2 is boolean")->obj)->boolval);

  TEST_ASSERT_EQUAL(True, (eval_program("true is boolean")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("true is string")->obj)->boolval);

  TEST_ASSERT_EQUAL(True, (eval_program("\"glug\" is string")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("\"glug\" is byte")->obj)->boolval);

  TEST_ASSERT_EQUAL(True, (eval_program("'c' is byte")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("'c' is int")->obj)->boolval);

  TEST_ASSERT_EQUAL(True, (eval_program("\"moo\" is string")->obj)->boolval);
  TEST_ASSERT_EQUAL(False, (eval_program("\"moo\" is int")->obj)->boolval);

  // TODO There's currently no way to do this for bytearray.
  // Want a proper list of types that includes keyword etc, so we can say type(while) etc.
  // Possible hook for documentation, as well.
}

void test_eval_type_of(void) {
  TEST_ASSERT_EQUAL_STRING("Int",        bytearray_to_c_str(eval_program("type(2)")->obj->bytearray));
  TEST_ASSERT_EQUAL_STRING("Float",      bytearray_to_c_str(eval_program("type(2.2)")->obj->bytearray));
  TEST_ASSERT_EQUAL_STRING("Bool",       bytearray_to_c_str(eval_program("type(true)")->obj->bytearray));
  TEST_ASSERT_EQUAL_STRING("Str",        bytearray_to_c_str(eval_program("type(\"2\")")->obj->bytearray));
  TEST_ASSERT_EQUAL_STRING("Byte Array", bytearray_to_c_str(eval_program("type(arr(2))")->obj->bytearray));
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
  char *program = "{ val l = list of Int { 1, 2, 3}\n"
                  "  l.length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(3, obj->intval);
}

void test_eval_list_val_eq(void) {
  char *program = "{ val a = list of Int { 1, 2, 3 }   \n"
                  "  val b = list of Int { 0, 1, 2, 3 }\n"
                  "  var i = 0                         \n"
                  "  if (a == b.tail()) then i = 1     \n"
                  "  if (a == b) then i = 0            \n"
                  "  i                                 \n"
                  "}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_get(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.get(0) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  char *program1= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.get(1) }";
  eval_result_t *result1 = eval_program(program1);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result1->err);
  TEST_ASSERT_EQUAL(2, result1->obj->intval);

  char *program2= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.get(2) }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(3, result2->obj->intval);

  char *program3 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.get(-1) }";
  eval_result_t *result3 = eval_program(program3);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result3->err);
  TEST_ASSERT_EQUAL(3, result3->obj->intval);

  char *program4= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.get(-2) }";
  eval_result_t *result4 = eval_program(program4);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result4->err);
  TEST_ASSERT_EQUAL(2, result4->obj->intval);

  char *program5= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.get(-3) }";
  eval_result_t *result5 = eval_program(program5);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result5->err);
  TEST_ASSERT_EQUAL(1, result5->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list of Int\n"
                   "  l.get(0) }";
  eval_result_t *result6 = eval_program(program6);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result6->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result6->obj->type);

  char *program7 = "{ val l = list of Int\n"
                   "  l.get(-1) }";
  eval_result_t *result7 = eval_program(program7);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result7->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result7->obj->type);

  char *program8 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.get(14) }";
  eval_result_t *result8 = eval_program(program8);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result8->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result8->obj->type);

  char *program9 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.get(-5) }";
  eval_result_t *result9 = eval_program(program9);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result9->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result9->obj->type);
}

void test_eval_list_val_head(void) {
  char *program = "{ val l = list of Int { 1, 2, 3}\n"
                  "  l.head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_tail_length(void) {
  char *program = "{ val l = list of Int { 1, 2, 3}\n"
                  "  l.tail().length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head(void) {
  char *program = "{ val l = list of Int { 1, 2, 3, 4, 5}\n"
                  "  l.slice(1, 3).head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(2, obj->intval);
}

void test_eval_list_val_slice_head_tail_length(void) {
  char *program = "{ val l = list of Int { 1, 2, 3, 4, 5}\n"
                  "  l.slice(1, 3).tail().length() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);

  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(1, obj->intval);
}

void test_eval_list_val_prepend(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.prepend(6)\n"
                  "  l.head() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list of Int\n"
                   "  l.prepend(8)\n"
                   "  l.head() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(8, result2->obj->intval);
}

void test_eval_list_val_append(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.append(6)\n"
                  "  l.get(3) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(6, result->obj->intval);

  char *program2 = "{ val l = list of Int\n"
                   "  l.append(8)\n"
                   "  l.head() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(8, result2->obj->intval);
}

void test_eval_list_val_remove_first(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeFirst() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(1, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list of Int\n"
                   "  l.removeFirst() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result2->obj->type);
}

void test_eval_list_val_remove_last(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeLast() }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(3, result->obj->intval);

  // Edge cases.
  char *program2 = "{ val l = list of Int\n"
                   "  l.removeLast() }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result2->obj->type);
}

void test_eval_list_val_remove_at(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeAt(0)\n"
                  "  l.get(0) }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);

  char *program1= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeAt(1)\n"
                  "  l.get(1) }";
  eval_result_t *result1 = eval_program(program1);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result1->err);
  TEST_ASSERT_EQUAL(3, result1->obj->intval);

  char *program2= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeAt(2)\n"
                  "  l.get(1) }";
  eval_result_t *result2 = eval_program(program2);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(2, result2->obj->intval);

  char *program3 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.removeAt(-1)\n"
                   "  l.get(-1) }";
  eval_result_t *result3 = eval_program(program3);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result3->err);
  TEST_ASSERT_EQUAL(2, result3->obj->intval);

  char *program4= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeAt(-2)\n"
                  "  l.get(1) }";
  eval_result_t *result4 = eval_program(program4);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result4->err);
  TEST_ASSERT_EQUAL(3, result4->obj->intval);

  char *program5= "{ val l = list of Int { 1, 2, 3 }\n"
                  "  l.removeAt(-3)\n"
                  "  l.get(0) }";
  eval_result_t *result5 = eval_program(program5);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result5->err);
  TEST_ASSERT_EQUAL(2, result5->obj->intval);

  // Edge cases.
  char *program6 = "{ val l = list of Int\n"
                   "  l.removeAt(0) }";
  eval_result_t *result6 = eval_program(program6);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result6->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result6->obj->type);

  char *program7 = "{ val l = list of Int\n"
                   "  l.removeAt(-1) }";
  eval_result_t *result7 = eval_program(program7);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result7->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result7->obj->type);

  char *program8 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.removeAt(14) }";
  eval_result_t *result8 = eval_program(program8);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result8->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result8->obj->type);

  char *program9 = "{ val l = list of Int { 1, 2, 3 }\n"
                   "  l.removeAt(-5) }";
  eval_result_t *result9 = eval_program(program9);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result9->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result9->obj->type);
}

void test_eval_list_in(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  2 in l }";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_arr_decl(void) {
  char *program = "{ val a = arr(12)\n"
                  "  a.length() }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(12, result->obj->intval);

  char *program2= "{ val a = arr(12)\n"
                  "  a[3] }";
  eval_result_t *result2= eval_program(program2);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_BYTE, result2->obj->type);
  TEST_ASSERT_EQUAL('\0', result2->obj->byteval);
}

void test_eval_arr_assign(void) {
  // Assign with int.
  char *program = "{ val a = arr(12)\n"
                  "  a[4] = 42\n"
                  "  a[4] }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL('*', result->obj->byteval);

  // Assign with char.
  char *program2= "{ val a = arr(12)\n"
                  "  a[3] = 'x'\n"
                  "  a[3] }";
  eval_result_t *result2= eval_program(program2);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result2->err);
  TEST_ASSERT_EQUAL(TYPE_BYTE, result2->obj->type);
  TEST_ASSERT_EQUAL('x', result2->obj->byteval);
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
  char *program = "{ val f = fn(x) { x + 1 } \n"
                  "  f(1) }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_INT, result->obj->type);
  TEST_ASSERT_EQUAL(2, result->obj->intval);
}

void test_eval_function_undef(void) {
  char *program = "nope(42)";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_FUNCTION_UNDEFINED, result->err);
  TEST_ASSERT_EQUAL(TYPE_NIL, result->obj->type);
}

void test_eval_function_wrong_args(void) {
  char *program = "{ val f = fn(x, y) { x + y } \n"
                  "  f(1) }";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_WRONG_ARG_COUNT, result->err);
}

void test_eval_function_return(void) {
  char *program = "{ val f = fn(x) {\n"
                  "    if x == 0 then return 42\n"
                  "    x\n"
                  "  } \n"
                  "  f(0)}";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_function_lexical_scope(void) {
  char *program = "{ val x = 42            \n"
                  "  val f = fn()  {   x } \n"
                  "  val g = fn(x) { f() } \n"
                  "  g(17)                 \n"
                  "}";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_block_scope(void) {
  char *program = "{ val x = 42       \n"
                  "  { { { x } } }    \n"
                  "}                  \n";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(42, result->obj->intval);
}

void test_eval_in_list(void) {
  char *program = "{ val l = list of Int { 1, 2, 3 }\n"
                  "  5 in l}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val l = list of Int { 1, 2, 3 }\n"
            "  2 in l}";
  result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_range(void) {
  char *program = "{ val s = \"glug\"\n"
                  "  0 in 1..s.length()}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val s = \"glug\"\n"
            "  3 in 1..s.length()}";
  result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_string(void) {
  char *program = "{ val s = \"Ethel\"\n"
                  "  'x' in s}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val s = \"Ethel\"\n"
            "  'e' in s}";
  result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_eval_in_bytearray(void) {
  char *program = "{ val a = arr(10)\n"
                  "  1 in a}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(False, result->obj->boolval);

  program = "{ val a = arr(10)\n"
            "  a[4] = 1 \n"
            "  1 in a}";
  result = eval_program(program);
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
  eval_result_t *result = eval_program(program);
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
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL_STRING("i lIke potatoes", bytearray_to_c_str(result->obj->bytearray));
}

void test_eval_arr_subscript_assign_int(void) {
  char *program = "{ val s = \"I like potatoes\"      \n"
                  "  s[0] = 85                        \n"
                  "  s[2] = 0x4c                      \n"
                  "  s                                \n"
                  "}";
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL_STRING("U Like potatoes", bytearray_to_c_str(result->obj->bytearray));
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
  eval_result_t *result = eval_program(program);
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(2, result->obj->intval);
}

void test_eval(void) {
  RUN_TEST(test_eval_calculator);
  RUN_TEST(test_eval_unary_minus);
  RUN_TEST(test_eval_assign_immutable);
  RUN_TEST(test_eval_assign_var);
  RUN_TEST(test_eval_assign_multiple);
  RUN_TEST(test_eval_del);
  RUN_TEST(test_eval_for_loop);
  RUN_TEST(test_eval_while_loop);
  RUN_TEST(test_eval_if_else);
  RUN_TEST(test_eval_if_else_nil);
  RUN_TEST(test_eval_if_else_assign);
  RUN_TEST(test_eval_if_else_assign_expr);
  RUN_TEST(test_eval_boolean_true);
  RUN_TEST(test_eval_boolean_false);
  RUN_TEST(test_eval_logical_not);
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
  RUN_TEST(test_eval_is_type);
  RUN_TEST(test_eval_type_of);
  RUN_TEST(test_eval_callable_abs);
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
  RUN_TEST(test_eval_block_scope);
  RUN_TEST(test_eval_in_list);
  RUN_TEST(test_eval_in_range);
  RUN_TEST(test_eval_in_string);
  RUN_TEST(test_eval_in_bytearray);
  RUN_TEST(test_eval_arr_subscript_cmp);
  RUN_TEST(test_eval_arr_subscript_assign_byte);
  RUN_TEST(test_eval_arr_subscript_assign_int);
  RUN_TEST(test_eval_str_compare);
}


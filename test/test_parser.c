#include <stdio.h>
#include "unity/unity.h"
#include "test_parser.h"
#include "../inc/err.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/parser.h"

void test_parse_empty(void) {
  char *program = "  ";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_EMPTY, ast->type);
}

void test_parse_add(void) {
  char *program = "1 + 2";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ADD, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->op_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->b)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->op_args->b)->intval);
}

void test_parse_sub(void) {
  char *program = "2 - 1";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_SUB, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->op_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->b)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->op_args->b)->intval);
}

void test_parse_mul(void) {
  char *program = "2 * 3";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_MUL, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->op_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->b)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->op_args->b)->intval);
}

void test_parse_div(void) {
  char *program = "3 / 6";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_DIV, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->op_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->b)->type);
  TEST_ASSERT_EQUAL(6, ((ast_expr_t*) ast->op_args->b)->intval);
}

void test_parse_assign(void) {
  char *program = "val x = 2 + 2";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_ADD, ((ast_expr_t*) ast->assignment->value)->type);
}

void test_parse_kv_assoc(void) {
  char *program = "'t' : 2";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_MAPS_TO, ast->type);
  TEST_ASSERT_EQUAL(AST_BYTE, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->op_args->b)->type);
}

void test_parse_kv_dict_init(void) {
  char *program = "val d = dict { 'a': 1, \"foo\": 2.2 }";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_DICT, ((ast_expr_t*) ast->assignment->value)->type);
  ast_dict_t *d = ast->assignment->value->dict;
  ast_expr_kv_list_t *kv1 = d->kv;
  TEST_ASSERT_EQUAL(AST_BYTE, ((ast_expr_t*) kv1->k)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) kv1->v)->type);
  ast_expr_kv_list_t *kv2 = d->kv->next;
  TEST_ASSERT_EQUAL(AST_STRING, ((ast_expr_t*) kv2->k)->type);
  TEST_ASSERT_EQUAL(AST_FLOAT, ((ast_expr_t*) kv2->v)->type);
}

void test_parse_if_else(void) {
  char *program = "if 1 then 2";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_IF_THEN, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->if_then_args->cond)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->if_then_args->pred)->type);
}

void test_parse_if_else_assign_expr(void) {
  char *program = "val x = if 1 then 2.0";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->if_then_args->cond)->type);
  TEST_ASSERT_EQUAL(AST_IF_THEN, ((ast_expr_t*) ast->if_then_args->pred)->type);
}

void test_parse_hex(void) {
  char *program = "val x = 0x8fff";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL(36863, ((ast_expr_t*) ast->assignment->value)->intval);
}

void test_parse_bin(void) {
  char *program = "val x = 0b1000";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL(8, ((ast_expr_t*) ast->assignment->value)->intval);
}

void test_parse_char(void) {
  char *program = "val c = 'c'";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BYTE, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL('c', ((ast_expr_t*) ast->assignment->value)->byteval);
}

void test_parse_string(void) {
  char *program = "val s = \"Ethel\"";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_STRING, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL_STRING("Ethel",
      bytearray_to_c_str(((ast_expr_t*) ast->assignment->value)->bytearray));
}

void test_parse_boolean_true(void) {
  char *program = "val x = true";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_TRUE(((ast_expr_t*) ast->assignment->value)->boolval);
}

void test_parse_boolean_false(void) {
  char *program = "val x = false";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_FALSE(((ast_expr_t*) ast->assignment->value)->boolval);
}

void test_parse_array_decl(void) {
  char *program = "val a = arr(12)";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BYTEARRAY_DECL, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->assignment->value->array_decl->size)->type);
}

void test_parse_seq_elem(void) {
  char *program = "a [ if (foo) then 0 else 1 ]";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_SUBSCRIPT, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(AST_IF_THEN_ELSE, ((ast_expr_t*) ast->op_args->b)->type);
}

void test_parse_seq_elem_access(void) {
  char *program = "val e = a[ 5 * 5 ]";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->op_args->a)->type);
  TEST_ASSERT_EQUAL(AST_SUBSCRIPT, ((ast_expr_t*) ast->op_args->b)->type);
}

void test_parse_seq_elem_assign(void) {
  char *program = "a[ 5 * 5 ] = 123";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_SUBSCRIPT, ((ast_expr_t*) ast->op_args->a)->type);
  ast_expr_t *ss = (ast_expr_t *) ast->op_args->a;
  ast_expr_t *val = (ast_expr_t *) ast->op_args->b;
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ss->op_args->a)->type);
  TEST_ASSERT_EQUAL(AST_MUL, ((ast_expr_t*) ss->op_args->b)->type);
  TEST_ASSERT_EQUAL(AST_INT, val->type);
}

void test_parse_member_of(void) {
  char *program = "val x = 3 in 0..10";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_IN, ((ast_expr_t*) ast->assignment->value)->type);
}

void test_parse_empty_func(void) {
  char *program = "val x = fn() { }";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_FUNCTION_DEF, ((ast_expr_t*) ast->assignment->value)->type);
  ast_func_def_t *f= (ast_func_def_t*) ast->assignment->value->func_def;
  TEST_ASSERT_NULL(f->argnames);
}

void test_parse_func(void) {
  char *program = "val x = fn(a, b, c) { val r = a + b + c\nr }";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_FUNCTION_DEF, ((ast_expr_t*) ast->assignment->value)->type);
  ast_func_def_t *f= (ast_func_def_t*) ast->assignment->value->func_def;
  TEST_ASSERT_EQUAL_STRING("a", bytearray_to_c_str(f->argnames->name));
  TEST_ASSERT_EQUAL_STRING("b", bytearray_to_c_str(f->argnames->next->name));
  TEST_ASSERT_EQUAL_STRING("c", bytearray_to_c_str(f->argnames->next->next->name));
  TEST_ASSERT_EQUAL(AST_IDENT, f->block_exprs->next->root->type);
}

void test_parse_func_call(void) {
  char *program = "val x = f(42)";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_FUNCTION_CALL, ((ast_expr_t*) ast->assignment->value)->type);
  ast_func_call_t *func_call = (ast_func_call_t*) ast->assignment->value->func_call;
  TEST_ASSERT_EQUAL(42, func_call->args->root->intval);
}

void test_parser(void) {
  RUN_TEST(test_parse_empty);
  RUN_TEST(test_parse_add);
  RUN_TEST(test_parse_sub);
  RUN_TEST(test_parse_mul);
  RUN_TEST(test_parse_div);
  RUN_TEST(test_parse_assign);
  RUN_TEST(test_parse_kv_assoc);
  RUN_TEST(test_parse_kv_dict_init);
  RUN_TEST(test_parse_if_else);
  RUN_TEST(test_parse_if_else_assign_expr);
  RUN_TEST(test_parse_char);
  RUN_TEST(test_parse_hex);
  RUN_TEST(test_parse_bin);
  RUN_TEST(test_parse_string);
  RUN_TEST(test_parse_boolean_true);
  RUN_TEST(test_parse_boolean_false);
  RUN_TEST(test_parse_array_decl);
  RUN_TEST(test_parse_seq_elem);
  RUN_TEST(test_parse_seq_elem_access);
  RUN_TEST(test_parse_seq_elem_assign);
  RUN_TEST(test_parse_member_of);
  RUN_TEST(test_parse_empty_func);
  RUN_TEST(test_parse_func);
  RUN_TEST(test_parse_func_call);
}


#include <stdio.h>
#include "unity/unity.h"
#include "test_parser.h"
#include "../inc/err.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/parser.h"

#define USING_PROGRAM(text) \
    char *program = text; \
    ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t)); \
    parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t)); \
    parse_program(program, ast, parse_result);
  
void test_parse_empty(void) {
  char *program = "  ";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_EMPTY, ast->type);
  mem_free(ast);
}

void test_parse_add(void) {
  char *program = "1 + 2";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ADD, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->b)->intval);
  mem_free(ast);
}

void test_parse_sub(void) {
  char *program = "2 - 1";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_SUB, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->binop_args->b)->intval);
  mem_free(ast);
}

void test_parse_mul(void) {
  char *program = "2 * 3";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_MUL, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->binop_args->b)->intval);
  mem_free(ast);
}

void test_parse_div(void) {
  char *program = "3 / 6";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_DIV, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(6, ((ast_expr_t*) ast->binop_args->b)->intval);
  mem_free(ast);
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
  mem_free(ast);
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
  mem_free(ast);
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
  mem_free(ast);
}

void test_parse_char(void) {
  char *program = "val c = 'c'";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_CHAR, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL('c', ((ast_expr_t*) ast->assignment->value)->charval);
  mem_free(ast);
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
  mem_free(ast);
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
  mem_free(ast);
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
  mem_free(ast);
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
  mem_free(ast);
}

void test_parse_seq_elem(void) {
  char *program = "a [ if (foo) then 0 else 1 ]";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_SEQ_ELEM, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->seq_elem->ident)->type);
  TEST_ASSERT_EQUAL(AST_IF_THEN_ELSE, ((ast_expr_t*) ast->seq_elem->index)->type);
}

void test_parse_seq_elem_access(void) {
  char *program = "val e = a[ 5 * 5 ]";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_SEQ_ELEM, ((ast_expr_t*) ast->assignment->value)->type);
}

void test_parse_seq_elem_assign(void) {
  char *program = "a[ 5 * 5 ] = 123";
  ast_expr_t *ast = mem_alloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = mem_alloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_REASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_SEQ_ELEM, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->assignment->value)->type);
}

void test_parser(void) {
  RUN_TEST(test_parse_empty);
  RUN_TEST(test_parse_add);
  RUN_TEST(test_parse_sub);
  RUN_TEST(test_parse_mul);
  RUN_TEST(test_parse_div);
  RUN_TEST(test_parse_assign);
  RUN_TEST(test_parse_if_else);
  RUN_TEST(test_parse_if_else_assign_expr);
  RUN_TEST(test_parse_char);
  RUN_TEST(test_parse_string);
  RUN_TEST(test_parse_boolean_true);
  RUN_TEST(test_parse_boolean_false);
  RUN_TEST(test_parse_array_decl);
  RUN_TEST(test_parse_seq_elem);
  RUN_TEST(test_parse_seq_elem_access);
  RUN_TEST(test_parse_seq_elem_assign);
}


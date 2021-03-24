#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unity/unity.h"
#include "test_parser.h"
#include "../inc/err.h"
#include "../inc/parser.h"

void test_parse_empty(void) {
  char *program = "  ";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_EMPTY, ast->type);
  free(ast);
}

void test_parse_add(void) {
  char *program = "1 + 2";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ADD, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->b)->intval);
  free(ast);
}

void test_parse_sub(void) {
  char *program = "2 - 1";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_SUB, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) ast->binop_args->b)->intval);
  free(ast);
}

void test_parse_mul(void) {
  char *program = "2 * 3";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_MUL, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->binop_args->b)->intval);
  free(ast);
}

void test_parse_div(void) {
  char *program = "3 / 6";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_DIV, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->a)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) ast->binop_args->a)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->binop_args->b)->type);
  TEST_ASSERT_EQUAL(6, ((ast_expr_t*) ast->binop_args->b)->intval);
  free(ast);
}

void test_parse_assign(void) {
  char *program = "x = 2 + 2";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_ADD, ((ast_expr_t*) ast->assignment->value)->type);
  free(ast);
}

void test_parse_if_else(void) {
  char *program = "if 1 then 2";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_IF_THEN, ast->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->if_then_args->cond)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) ast->if_then_args->pred)->type);
  free(ast);
}

void test_parse_if_else_assign_expr(void) {
  char *program = "x = if 1 then 2.0";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->if_then_args->cond)->type);
  TEST_ASSERT_EQUAL(AST_IF_THEN, ((ast_expr_t*) ast->if_then_args->pred)->type);
  free(ast);
}

void test_parse_char(void) {
  char *program = "c = 'c'";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_CHAR, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL('c', ((ast_expr_t*) ast->assignment->value)->charval);
  free(ast);
}

void test_parse_string(void) {
  char *program = "s = \"Ethel\"";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_STRING, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_EQUAL_STRING("Ethel", ((ast_expr_t*) ast->assignment->value)->stringval);
  free(ast);
}

void test_parse_boolean_true(void) {
  char *program = "x = true";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_TRUE(((ast_expr_t*) ast->assignment->value)->intval);
  free(ast);
}

void test_parse_boolean_false(void) {
  char *program = "x = false";
  ast_expr_t *ast = malloc(sizeof(ast_expr_t));
  parse_result_t *parse_result = malloc(sizeof(parse_result_t));
  parse_program(program, ast, parse_result);

  TEST_ASSERT_EQUAL(NO_ERROR, parse_result->err);
  TEST_ASSERT_EQUAL(AST_ASSIGN, ast->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) ast->assignment->ident)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) ast->assignment->value)->type);
  TEST_ASSERT_FALSE(((ast_expr_t*) ast->assignment->value)->intval);
  free(ast);
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
}


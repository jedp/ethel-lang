#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_parser.h"
#include "../inc/parser.h"

void test_parse_empty(void) {
  char *program = "  ";
  ast_expr_t *expr = parse_program(program);

  TEST_ASSERT_EQUAL(AST_EMPTY, expr->type);
}

void test_parse_add(void) {
  char *program = "1 + 2";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ADD, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) expr->e1)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) expr->e2)->intval);
}

void test_parse_sub(void) {
  char *program = "2 - 1";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_SUB, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) expr->e1)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL(1, ((ast_expr_t*) expr->e2)->intval);
}

void test_parse_mul(void) {
  char *program = "2 * 3";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_MUL, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(2, ((ast_expr_t*) expr->e1)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) expr->e2)->intval);
}

void test_parse_div(void) {
  char *program = "3 / 6";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_DIV, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(3, ((ast_expr_t*) expr->e1)->intval);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL(6, ((ast_expr_t*) expr->e2)->intval);
}

void test_parse_assign(void) {
  char *program = "x = 2 + 2";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_ADD, ((ast_expr_t*) expr->e2)->type);
}

void test_parse_if_else(void) {
  char *program = "if 1 then 2";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_IF_THEN, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e2)->type);
}

void test_parse_if_else_assign_expr(void) {
  char *program = "x = if 1 then 2.0";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_IF_THEN, ((ast_expr_t*) expr->e2)->type);
}

void test_parse_char(void) {
  char *program = "c = 'c'";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_CHAR, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL('c', ((ast_expr_t*) expr->e2)->charval);
}

void test_parse_string(void) {
  char *program = "s = \"Ethel\"";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_STRING, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_EQUAL_STRING("Ethel", ((ast_expr_t*) expr->e2)->stringval);
}

void test_parse_boolean_true(void) {
  char *program = "x = true";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_TRUE(((ast_expr_t*) expr->e2)->intval);
}

void test_parse_boolean_false(void) {
  char *program = "x = false";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ASSIGN, expr->type);
  TEST_ASSERT_EQUAL(AST_IDENT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_BOOLEAN, ((ast_expr_t*) expr->e2)->type);
  TEST_ASSERT_FALSE(((ast_expr_t*) expr->e2)->intval);
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


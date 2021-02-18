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

/* Ensure we parse the rest of the expr after parsing a term. */
void test_parse_add_multiple(void) {
  char *program = "1 + 2 + 3";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_ADD, expr->type);

  // The INT leaf.
  ast_expr_t *e1 = (ast_expr_t*) expr->e1;
  TEST_ASSERT_EQUAL(AST_INT, e1->type);
  TEST_ASSERT_EQUAL(1, e1->intval);

  // The ADD node.
  ast_expr_t *e2 = (ast_expr_t*) expr->e2;
  TEST_ASSERT_EQUAL(AST_ADD, e2->type);

  // The ADD node's two INT leaves.
  ast_expr_t *e2_left = (ast_expr_t*) e2->e1;
  TEST_ASSERT_EQUAL(AST_INT, e2_left->type);
  TEST_ASSERT_EQUAL(2, e2_left->intval);
  ast_expr_t *e2_right = (ast_expr_t*) e2->e2;
  TEST_ASSERT_EQUAL(AST_INT, e2_right->type);
  TEST_ASSERT_EQUAL(3, e2_right->intval);
}

/* Ensure we parse the rest of the expr after parsing a factor. */
void test_parse_mul_multiple(void) {
  char *program = "2 * 4 * 6";

  ast_expr_t *expr = parse_program(program);
  TEST_ASSERT_EQUAL(AST_MUL, expr->type);

  // The INT leaf.
  ast_expr_t *e1 = (ast_expr_t*) expr->e1;
  TEST_ASSERT_EQUAL(AST_INT, e1->type);
  TEST_ASSERT_EQUAL(2, e1->intval);

  // The MUL node.
  ast_expr_t *e2 = (ast_expr_t*) expr->e2;
  TEST_ASSERT_EQUAL(AST_MUL, e2->type);

  // The MUL node's two INT leaves.
  ast_expr_t *e2_left = (ast_expr_t*) e2->e1;
  TEST_ASSERT_EQUAL(AST_INT, e2_left->type);
  TEST_ASSERT_EQUAL(4, e2_left->intval);
  ast_expr_t *e2_right = (ast_expr_t*) e2->e2;
  TEST_ASSERT_EQUAL(AST_INT, e2_right->type);
  TEST_ASSERT_EQUAL(6, e2_right->intval);
}

/* Ensure terms and factors reflect the correct order of operations.  */
void test_parse_priority(void) {
  char *program = "1 + 2 * 3 + 4";

  ast_expr_t *expr = parse_program(program);

  /*
  (ADD
    (INT 1)
    (ADD
      (MUL
        (INT 2)
        (INT 3))
      (INT 4)))
  */

  TEST_ASSERT_EQUAL(AST_ADD, expr->type);
  TEST_ASSERT_EQUAL(AST_INT, ((ast_expr_t*) expr->e1)->type);
  TEST_ASSERT_EQUAL(AST_ADD, ((ast_expr_t*) expr->e2)->type);
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

void test_parser(void) {
  RUN_TEST(test_parse_empty);
  RUN_TEST(test_parse_add);
  RUN_TEST(test_parse_sub);
  RUN_TEST(test_parse_mul);
  RUN_TEST(test_parse_div);
  RUN_TEST(test_parse_add_multiple);
  RUN_TEST(test_parse_mul_multiple);
  RUN_TEST(test_parse_priority);
  RUN_TEST(test_parse_assign);
  RUN_TEST(test_parse_if_else);
  RUN_TEST(test_parse_if_else_assign_expr);
}


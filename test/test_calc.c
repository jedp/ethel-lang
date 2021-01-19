#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_calc.h"
#include "lexer.h"
#include "calc.h"

void test_digit(void) {
  char *expr = "2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(2, parse_start(&lexer));
}

void test_digit_neg(void) {
  char *expr = "-2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(-2, parse_start(&lexer));
}

void test_add(void) {
  char *expr = "1 + 1";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(2, parse_start(&lexer));
}

void test_sub(void) {
  char *expr = "4-2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(2, parse_start(&lexer));
}

void test_mul(void) {
  char *expr = "2 * 4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(8, parse_start(&lexer));
}

void test_div(void) {
  char *expr = "4 / 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(2, parse_start(&lexer));
}

void test_precedence(void) {
  char *expr = "(1 + 2) * (6 / 3)";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(6, parse_start(&lexer));
}

void test_compound(void) {
  char *expr = "(3 * (2 - 1)) + (10 / 5) - 1";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(4, parse_start(&lexer));
}

void test_compound_with_negatives(void) {
  char *expr = "(3 * (-2 - -1)) + (10 / -5) - 1";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(-6, parse_start(&lexer));
}

void test_spacing(void) {
  char *expr = "(3*(2-1)) + (4+-1)*-4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(-9, parse_start(&lexer));
}

void test_calc(void) {
  RUN_TEST(test_digit);
  RUN_TEST(test_digit_neg);
  RUN_TEST(test_add);
  RUN_TEST(test_sub);
  RUN_TEST(test_mul);
  RUN_TEST(test_div);
  RUN_TEST(test_precedence);
  RUN_TEST(test_compound);
  RUN_TEST(test_compound_with_negatives);
  RUN_TEST(test_spacing);
}


#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_lexer.h"
#include "lexer.h"

void test_lex_error(void) {
  char *expr = "if 42 & moo";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  advance(&lexer);
  advance(&lexer);
  advance(&lexer);

  TEST_ASSERT_EQUAL(_EOF, lexer.token.tag);
  TEST_ASSERT_EQUAL(6, lexer.err);
}

void test_lex_word(void) {
  char *expr = "for i in 1 to 10 step 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  token_t expected[] = {
    { .tag = FOR,     .string = "for" },
    { .tag = VARNAME, .string = "i" },
    { .tag = IN,      .string = "in" },
    { .tag = INT,     .intval = 1 },
    { .tag = TO,      .string = "to" },
    { .tag = INT,     .intval = 10 },
    { .tag = STEP,    .string = "step" },
    { .tag = INT,     .intval = 2 },
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    advance(&lexer);
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i].tag, lexer.token.tag);
    if (lexer.token.tag == INT) {
      TEST_ASSERT_EQUAL(expected[i].intval, lexer.token.intval);
    } else {
      TEST_ASSERT(!strcmp(expected[i].string, lexer.token.string));
    }
  }
}

void test_lex_int(void) {
  char *expr = "3120";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  advance(&lexer);

  TEST_ASSERT_EQUAL(INT, lexer.token.tag);
  TEST_ASSERT_EQUAL(3120, lexer.token.intval);
}

void test_lex_float(void) {
  char *expr = "3.1415";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  advance(&lexer);

  TEST_ASSERT_EQUAL(FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 3.1415, lexer.token.floatval);
}

void test_lex_float_no_leading_decimal(void) {
  char *expr = ".125";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  advance(&lexer);

  TEST_ASSERT_EQUAL(FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 0.125, lexer.token.floatval);
}

void test_lex_arithmetic(void) {
  char *expr = "123 + 2345.67 * 3 - .42 / 5";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    INT, ADD,
    FLOAT, MUL,
    INT, SUB,
    FLOAT, DIV,
    INT, _EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    advance(&lexer);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
  }
}

void test_lex_inequality(void) {
  char *expr = "if a < 1 and b <= 2 and c > 3 and d >= 4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    IF, VARNAME, LT, INT,
    AND, VARNAME, LE, INT,
    AND, VARNAME, GT, INT,
    AND, VARNAME, GE, INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    advance(&lexer);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
  }
}

void test_lex_parens(void) {
  char *expr = "(1.24 * ( 3.5-2+ 4 ) + 1) * 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    LPAREN, FLOAT, MUL,
    LPAREN, FLOAT, SUB, INT, ADD, INT, RPAREN,
    ADD, INT, RPAREN,
    MUL, INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    advance(&lexer);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
  }
}

void test_lexer(void) {
  RUN_TEST(test_lex_error);
  RUN_TEST(test_lex_word);
  RUN_TEST(test_lex_int);
  RUN_TEST(test_lex_float);
  RUN_TEST(test_lex_float_no_leading_decimal);
  RUN_TEST(test_lex_arithmetic);
  RUN_TEST(test_lex_inequality);
  RUN_TEST(test_lex_parens);
}


#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_lexer.h"
#include "../inc/lexer.h"

void test_lex_error(void) {
  char *expr = "if 42 & moo";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  advance(&lexer); // eat "if"
  advance(&lexer); // eat "42"

  TEST_ASSERT_EQUAL(TAG_EOF, lexer.token.tag);
  TEST_ASSERT_EQUAL(6, lexer.err);
}

void test_lex_word(void) {
  char *expr = "for i in 1 to 10 step 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  token_t expected[] = {
    { .tag = TAG_FOR,     .string = "for" },
    { .tag = TAG_IDENT,   .string = "i" },
    { .tag = TAG_IN,      .string = "in" },
    { .tag = TAG_INT,     .intval = 1 },
    { .tag = TAG_TO,      .string = "to" },
    { .tag = TAG_INT,     .intval = 10 },
    { .tag = TAG_STEP,    .string = "step" },
    { .tag = TAG_INT,     .intval = 2 },
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i].tag, lexer.token.tag);
    if (lexer.token.tag == TAG_INT) {
      TEST_ASSERT_EQUAL(expected[i].intval, lexer.token.intval);
    } else {
      TEST_ASSERT(!strcmp(expected[i].string, lexer.token.string));
    }
    advance(&lexer);
  }
}

void test_lex_int(void) {
  char *expr = "3120";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(TAG_INT, lexer.token.tag);
  TEST_ASSERT_EQUAL(3120, lexer.token.intval);
}

void test_lex_float(void) {
  char *expr = "3.1415";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(TAG_FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 3.1415, lexer.token.floatval);
}

void test_lex_float_no_leading_decimal(void) {
  char *expr = ".125";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  TEST_ASSERT_EQUAL(TAG_FLOAT, lexer.token.tag);
  TEST_ASSERT_EQUAL((float) 0.125, lexer.token.floatval);
}

void test_lex_arithmetic(void) {
  char *expr = "123 + 2345.67 * 3 - .42 / 5";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_INT, TAG_PLUS,
    TAG_FLOAT, TAG_TIMES,
    TAG_INT, TAG_MINUS,
    TAG_FLOAT, TAG_DIVIDE,
    TAG_INT, TAG_EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_arithmetic_no_spaces(void) {
  char *expr = "123+2345.67*3-.42/5";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_INT, TAG_PLUS,
    TAG_FLOAT, TAG_TIMES,
    TAG_INT, TAG_MINUS,
    TAG_FLOAT, TAG_DIVIDE,
    TAG_INT, TAG_EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality(void) {
  char *expr = "if a < 1 and b <= 2 and c > 3 and d >= 4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_IF, TAG_IDENT, TAG_LT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_LE, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality_no_spaces(void) {
  char *expr = "if a<1 and b<=2 and c>3 and d>=4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_IF, TAG_IDENT, TAG_LT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_LE, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_inequality_nested_expressions(void) {
  char *expr = "if (a < 1) and (b<=(2*4)) and c>3 and d>=4";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_IF, TAG_LPAREN, TAG_IDENT, TAG_LT, TAG_INT, TAG_RPAREN,
    TAG_AND, TAG_LPAREN, TAG_IDENT, TAG_LE,
      TAG_LPAREN, TAG_INT, TAG_TIMES, TAG_INT, TAG_RPAREN, TAG_RPAREN,
    TAG_AND, TAG_IDENT, TAG_GT, TAG_INT,
    TAG_AND, TAG_IDENT, TAG_GE, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_parens(void) {
  char *expr = "(1.24 * ( 3.5-2+ 4 ) + 1) * 2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_LPAREN, TAG_FLOAT, TAG_TIMES,
    TAG_LPAREN, TAG_FLOAT, TAG_MINUS, TAG_INT, TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_TIMES, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_parens_no_spaces(void) {
  char *expr = "(1.24*(3.5-2+4)+1)*2";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    TAG_LPAREN, TAG_FLOAT, TAG_TIMES,
    TAG_LPAREN, TAG_FLOAT, TAG_MINUS, TAG_INT, TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_PLUS, TAG_INT, TAG_RPAREN,
    TAG_TIMES, TAG_INT
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lex_assign(void) {
  char *expr = "a = 3";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = { TAG_IDENT, TAG_ASSIGN, TAG_INT };
  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], lexer.token.tag);
    advance(&lexer);
  }
}

void test_lexer(void) {
  RUN_TEST(test_lex_error);
  RUN_TEST(test_lex_word);
  RUN_TEST(test_lex_int);
  RUN_TEST(test_lex_float);
  RUN_TEST(test_lex_float_no_leading_decimal);
  RUN_TEST(test_lex_arithmetic);
  RUN_TEST(test_lex_arithmetic_no_spaces);
  RUN_TEST(test_lex_inequality);
  RUN_TEST(test_lex_inequality_nested_expressions);
  RUN_TEST(test_lex_parens);
  RUN_TEST(test_lex_parens_no_spaces);
  RUN_TEST(test_lex_assign);
}


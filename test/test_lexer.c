#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_lexer.h"
#include "lexer.h"

void test_lex_error(void) {
  char *expr = "if 42 & moo";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  lexer_next(&lexer);
  lexer_next(&lexer);
  lexer_next(&lexer);

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
    { .tag = INT,     .value = 1 },
    { .tag = TO,      .string = "to" },
    { .tag = INT,     .value = 10 },
    { .tag = STEP,    .string = "step" },
    { .tag = INT,     .value = 2 },
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    token_t *t = lexer_next(&lexer);
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i].tag, t->tag);
    if (t->tag == INT) {
      TEST_ASSERT_EQUAL(expected[i].value, t->value);
    } else {
      TEST_ASSERT(!strcmp(expected[i].string, t->string));
    }
  }
}

void test_lex_num(void) {
  char *expr = "3120";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  token_t *token = lexer_next(&lexer);

  TEST_ASSERT_EQUAL(INT, token->tag);
  TEST_ASSERT_EQUAL(3120, token->value);
}

void test_lex_arithmetic(void) {
  char *expr = "123 + 2345 * 3 - 42 / 5";
  lexer_t lexer;
  lexer_init(&lexer, expr, strlen(expr));

  int expected[] = {
    INT, ADD,
    INT, MUL,
    INT, SUB,
    INT, DIV,
    INT, _EOF
  };

  for (int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
    TEST_ASSERT_EQUAL(-1, lexer.err);
    TEST_ASSERT_EQUAL(expected[i], (lexer_next(&lexer))->tag);
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
    TEST_ASSERT_EQUAL(expected[i], (lexer_next(&lexer))->tag);
  }
}

void test_lexer(void) {
  RUN_TEST(test_lex_error);
  RUN_TEST(test_lex_word);
  RUN_TEST(test_lex_num);
  RUN_TEST(test_lex_arithmetic);
  RUN_TEST(test_lex_inequality);
}


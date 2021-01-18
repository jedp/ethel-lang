#include <stdio.h>
#include <string.h>
#include "unity/unity.h"
#include "test_parser.h"
#include "parser.h"

void test_parse_numeric_expr(void) {
  char *expr = "1 + 2 / 3.0 * 4 - 5";

  // Exits with no error.
  TEST_ASSERT_EQUAL(0, parse_program(expr));
}

void test_parser(void) {
  RUN_TEST(test_parse_numeric_expr);
}


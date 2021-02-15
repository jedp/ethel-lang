#include "unity/unity.h"
#include "test_lexer.h"
#include "test_parser.h"
#include "test_eval.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  test_lexer();
  test_parser();
  test_eval();

  UNITY_END();
}


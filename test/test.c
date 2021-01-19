#include "unity/unity.h"
#include "test_map.h"
#include "test_lexer.h"
#include "test_calc.h"
#include "test_parser.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  test_map();
  test_lexer();
  test_calc();
  test_parser();

  UNITY_END();
}


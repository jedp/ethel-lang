#include "unity/unity.h"
#include "test_mem.h"
#include "test_str.h"
#include "test_bytearray.h"
#include "test_lexer.h"
#include "test_parser.h"
#include "test_eval.h"
#include "test_env.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  test_mem();
  test_str();
  test_bytearray();
  test_lexer();
  test_parser();
  test_eval();
  test_env();

  UNITY_END();
}


#include "unity/unity.h"
#include "test_mem.h"
#include "test_str.h"
#include "test_range.h"
#include "test_list.h"
#include "test_bytearray.h"
#include "test_lexer.h"
#include "test_parser.h"
#include "test_eval.h"
#include "test_hash.h"
#include "test_env.h"
#include "test_examples.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  test_mem();
  test_str();
  test_range();
  test_list();
  test_bytearray();
  test_lexer();
  test_parser();
  test_eval();
  test_hash();
  test_env();
  test_examples();

  UNITY_END();
}


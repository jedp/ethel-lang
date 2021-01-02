#include "unity/unity.h"
#include "test_map.h"
#include "test_lexer.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  test_map();
  test_lexer();

  UNITY_END();
}


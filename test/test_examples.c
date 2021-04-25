#include <stdio.h>
#include "unity/unity.h"
#include "test_examples.h"
#include "../inc/str.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define BA(x) (c_str_to_bytearray(x))

static eval_result_t *eval_program(char* program) {
  env_t env;
  env_init(&env);

  enter_scope(&env);

  return eval(&env, program);

  leave_scope(&env);
}

void test_ex_fibonacci(void) {
  char *program = "{ val fib = fn(x) {          \n"
                  "    if x <= 0 then return 0  \n"
                  "    if x == 1 then return 1  \n"
                  "    fib(x - 1) + fib(x - 2)  \n"
                  "  }                          \n"
                  "  fib(10)                    \n"
                  "}";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(55, result->obj->intval);
}

void test_ex_quicksort(void) {
  char *program = "{ val quicksort = fn(a, lo, hi) {  \n"
                  "    if lo < hi then {              \n"
                  "      var p = partition(a, lo, hi) \n"
                  "      quicksort(a, lo, p-1)        \n"
                  "      quicksort(a, p+1, hi)        \n"
                  "    }                              \n"
                  "  }                                \n"
                  "  val partition = fn(a, lo, hi) {  \n"
                  "    val pivot = a[hi]              \n"
                  "    var i = lo                     \n"
                  "    for j in lo..hi {              \n"
                  "      if s[j] < pivot then {       \n"
                  "        swap(a, i, j)              \n"
                  "        i = i + 1                  \n"
                  "      }                            \n"
                  "    }                              \n"
                  "    swap(a, i, hi)                 \n"
                  "    i                              \n"
                  "  }                                \n"
                  "  val swap = fn(a, i, j) {         \n"
                  "     val temp = a[i]               \n"
                  "     a[i] = a[j]                   \n"
                  "     a[j] = temp                   \n"
                  "  }                                \n"
                  "  val s = \"I like pie!\"          \n"
                  "  quicksort(s, 0, s.length() - 1)  \n"
                  "  s                                \n"
                  "}";
  eval_result_t *result = eval_program(program);

  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL_STRING("  !Ieeiiklp", bytearray_to_c_str(result->obj->bytearray));
}

void test_examples(void) {
  RUN_TEST(test_ex_fibonacci);
  RUN_TEST(test_ex_quicksort);
}

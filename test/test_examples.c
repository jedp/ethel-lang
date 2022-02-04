#include "util.h"
#include "unity/unity.h"
#include "test_examples.h"
#include "../inc/mem.h"
#include "../inc/type.h"
#include "../inc/str.h"
#include "../inc/rand.h"

void test_ex_fibonacci(void) {
    char *program = "{ val fib = fn(x) {          \n"
                    "    if x <= 0 then return 0  \n"
                    "    if x == 1 then return 1  \n"
                    "    fib(x - 1) + fib(x - 2)  \n"
                    "  }                          \n"
                    "  fib(10)                    \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

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
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
    TEST_ASSERT_EQUAL_STRING("  !Ieeiiklp", bytearray_to_c_str(result->obj->bytearray));
}

void test_ex_shuffle(void) {
    rand32_init();
    char *program = "{ val deck = list {                                \n"
                    "  \"A\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\", \n"
                    "  \"8\", \"9\", \"10\", \"J\", \"Q\", \"K\"        \n"
                    "  }                                                \n"
                    "  val shuffle = fn(cards) {                        \n"
                    "    for i in 0..cards.length()-1 {                 \n"
                    "      val n = rand(cards.length() - i) + i         \n"
                    "      val temp = cards[i]                          \n"
                    "      cards[i] = cards[n]                          \n"
                    "      cards[n] = temp                              \n"
                    "    }                                              \n"
                    "  }                                                \n"
                    "  shuffle(deck)                                    \n"
                    "  val expected = list {                            \n"
                    "  \"3\", \"10\", \"6\", \"J\", \"9\", \"A\", \"K\",\n"
                    "  \"5\", \"2\", \"7\", \"4\", \"Q\", \"8\"         \n"
                    "  }                                                \n"
                    "  deck == expected                                 \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
    TEST_ASSERT_EQUAL(True, result->obj->boolval);
}

void test_100_doors(void) {
    char *program = "{ val doors = arr(100)            \n"
                    "  for i in 0..99 {                \n"
                    "    for j in 0..99 step i+1 {     \n"
                    "      doors[j] = doors[j] ^ 1     \n"
                    "    }                             \n"
                    "  }                               \n"
                    "  ; Sum of doors that are open    \n"
                    "  ; is sum of squares, or 285.    \n"
                    "  var n = 0                       \n"
                    "  for i in 0..99 {                \n"
                    "    if doors[i] then n = n + i    \n"
                    "  }                               \n"
                    "  n                               \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(result->obj));
    TEST_ASSERT_EQUAL(285, result->obj->intval);
}

void test_examples(void) {
    RUN_TEST(test_ex_fibonacci);
    RUN_TEST(test_ex_quicksort);
    RUN_TEST(test_ex_shuffle);
    RUN_TEST(test_100_doors);
}

#include "util.h"
#include "unity/unity.h"
#include "test_closure.h"
#include "../inc/mem.h"

void test_return_closure(void) {
   char *program = "{                    \n"
                   "  val f = fn(x) {    \n"
                   "    val g = fn(y) {  \n"
                   "      x + y          \n"
                   "     }               \n"
                   "  }                  \n"
                   "  val closure = f(5) \n"
                   "  closure(10)        \n"
                   "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    obj_t *obj = result->obj;
    TEST_ASSERT_EQUAL(15, obj->intval);
}

void test_return_closure_unbound(void) {
    char *program = "{                    \n"
                    "  val f = fn(x) {    \n"
                    "    val g = fn(y) {  \n"
                    "      x + y          \n"
                    "     }               \n"
                    "  }                  \n"
                    "  f(5)(10)           \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    obj_t *obj = result->obj;
    TEST_ASSERT_EQUAL(15, obj->intval);
}

void test_return_anonymous_closure(void) {
    char *program = "{                    \n"
                    "  val f = fn(x) {    \n"
                    "    fn(y) { x + y }  \n"
                    "  }                  \n"
                    "  val closure = f(5) \n"
                    "  closure(10)        \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    obj_t *obj = result->obj;
    TEST_ASSERT_EQUAL(15, obj->intval);
}

void test_return_anonymous_closure_unbound(void) {
    char *program = "{                    \n"
                    "  val f = fn(x) {    \n"
                    "    fn(y) { x + y }  \n"
                    "  }                  \n"
                    "  f(5)(10)           \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    obj_t *obj = result->obj;
    TEST_ASSERT_EQUAL(15, obj->intval);
}

void test_closure(void) {
    RUN_TEST(test_return_closure);
    RUN_TEST(test_return_closure_unbound);
    RUN_TEST(test_return_anonymous_closure);
    RUN_TEST(test_return_anonymous_closure_unbound);
}
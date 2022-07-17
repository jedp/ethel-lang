#include "util.h"
#include "unity/unity.h"
#include "../inc/mem.h"
#include "../inc/type.h"
#include "../inc/str.h"
#include "../inc/heap.h"
#include "../inc/gc.h"

#define NAME(s) (c_str_to_bytearray(s))

void gc_primitives(void) {
    env_t env;
    env_init(&env);
    enter_scope(&env);

    obj_t *i = int_obj(42);
    // This object will always be in scope.
    put_env(&env, NAME("keep-int"), i, F_NONE);
    put_env(&env, NAME("keep-bool"), boolean_obj(1), F_NONE);
    put_env(&env, NAME("keep-float"), float_obj(4.2), F_NONE);
    put_env(&env, NAME("keep-byte"), byte_obj(0x0f), F_NONE);
    gc(&env);
    int init_free = get_heap_info()->bytes_free;

    // The object is unreachable by the env after we leave its scope.
    enter_scope(&env);
    put_env(&env, NAME("cull-int"), int_obj(17), F_NONE);
    put_env(&env, NAME("cull-bool"), boolean_obj(1), F_NONE);
    put_env(&env, NAME("cull-float"), float_obj(1.3), F_NONE);
    put_env(&env, NAME("cull-byte"), byte_obj(0x0a), F_NONE);
    leave_scope(&env);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&env);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-int"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-bool"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-float"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-byte"))));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&env, NAME("keep-int"))));
    TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(get_env(&env, NAME("keep-bool"))));
    TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(get_env(&env, NAME("keep-float"))));
    TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(get_env(&env, NAME("keep-byte"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_bytearray(void) {
    env_t env;
    env_init(&env);
    enter_scope(&env);

    put_env(&env, NAME("keep-bytearray"), bytearray_obj(0, NULL), F_NONE);
    gc(&env);
    int init_free = get_heap_info()->bytes_free;

    // Will be unreachable after we exit scope.
    enter_scope(&env);
    put_env(&env, NAME("cull-bytearray"), bytearray_obj(0, NULL), F_NONE);
    leave_scope(&env);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&env);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-bytearray"))));
    TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, TYPEOF(get_env(&env, NAME("keep-bytearray"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_string(void) {
    env_t env;
    env_init(&env);
    enter_scope(&env);

    put_env(&env, NAME("keep-string"), string_obj(NAME("Hi")), F_NONE);
    gc(&env);
    int init_free = get_heap_info()->bytes_free;

    // Will be unreachable after we exit scope.
    enter_scope(&env);
    put_env(&env, NAME("cull-string"), string_obj(NAME("Bye")), F_NONE);
    leave_scope(&env);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&env);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&env, NAME("cull-string"))));
    TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(get_env(&env, NAME("keep-string"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_scope(void) {
    // Regression test. Ensure the env stack can be cleanly unwound.
    char *program = "{ val f = fn(x) {            \n"
                    "    if x <= 1 then return 1  \n"
                    "    f(x-1)                   \n"
                    "  }                          \n"
                    "  f(20)                      \n"
                    "}";
    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    eval_program(program, result);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
}

void test_gc(void) {
    RUN_TEST(gc_primitives);
    RUN_TEST(gc_bytearray);
    RUN_TEST(gc_string);
    RUN_TEST(gc_scope);
}

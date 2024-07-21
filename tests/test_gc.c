#include "util.h"
#include "unity/unity.h"
#include "../inc/mem.h"
#include "../inc/type.h"
#include "../inc/str.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/heap.h"
#include "../inc/gc.h"

#define NAME(s) (c_str_to_bytearray(s))

static gc_header_t *decl(obj_t *obj) {
    obj->hdr.flags |= F_ENV_DECLARATION;
    return (gc_header_t *) obj;
}

void gc_primitives(void) {
    interp_t interp;
    interp_init(&interp);

    obj_t *i = int_obj(42);
    // This object will always be in scope.
    put_env(&interp, NAME("keep-int"), decl(i));
    put_env(&interp, NAME("keep-bool"), decl(boolean_obj(1)));
    put_env(&interp, NAME("keep-float"), decl(float_obj(4.2)));
    put_env(&interp, NAME("keep-byte"), decl(byte_obj(0x0f)));

    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&interp, NAME("keep-int"))));
    TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(get_env(&interp, NAME("keep-bool"))));
    TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(get_env(&interp, NAME("keep-float"))));
    TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(get_env(&interp, NAME("keep-byte"))));

    gc(&interp);
    int init_free = get_heap_info()->bytes_free;

    // The object is unreachable by the env after we leave its scope.
    enter_scope(&interp);
    put_env(&interp, NAME("cull-int"), decl(int_obj(17)));
    put_env(&interp, NAME("cull-bool"), decl(boolean_obj(1)));
    put_env(&interp, NAME("cull-float"), decl(float_obj(1.3)));
    put_env(&interp, NAME("cull-byte"), decl(byte_obj(0x0a)));
    leave_scope(&interp);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&interp);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-int"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-bool"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-float"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-byte"))));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&interp, NAME("keep-int"))));
    TEST_ASSERT_EQUAL(TYPE_BOOLEAN, TYPEOF(get_env(&interp, NAME("keep-bool"))));
    TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF(get_env(&interp, NAME("keep-float"))));
    TEST_ASSERT_EQUAL(TYPE_BYTE, TYPEOF(get_env(&interp, NAME("keep-byte"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_bytearray(void) {
    interp_t interp;
    interp_init(&interp);

    put_env(&interp, NAME("keep-bytearray"), decl(bytearray_obj(0, NULL)));
    gc(&interp);
    int init_free = get_heap_info()->bytes_free;

    // Will be unreachable after we exit scope.
    enter_scope(&interp);
    put_env(&interp, NAME("cull-bytearray"), decl(bytearray_obj(0, NULL)));
    leave_scope(&interp);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&interp);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-bytearray"))));
    TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, TYPEOF(get_env(&interp, NAME("keep-bytearray"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_string(void) {
    interp_t interp;
    interp_init(&interp);

    put_env(&interp, NAME("keep-string"), decl(string_obj(NAME("Hi"))));
    gc(&interp);
    int init_free = get_heap_info()->bytes_free;

    // Will be unreachable after we exit scope.
    enter_scope(&interp);
    put_env(&interp, NAME("cull-string"), decl(string_obj(NAME("Bye"))));
    leave_scope(&interp);

    // There is garbage.
    int mid_free = get_heap_info()->bytes_free;
    gc(&interp);

    int final_free = get_heap_info()->bytes_free;

    // Object accessibility is correct.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("cull-string"))));
    TEST_ASSERT_EQUAL(TYPE_STRING, TYPEOF(get_env(&interp, NAME("keep-string"))));

    // We actually cleaned garbage up.
    TEST_ASSERT_GREATER_THAN(mid_free, final_free);
    TEST_ASSERT_EQUAL(init_free, final_free);
}

void gc_list(void) {
    interp_t interp;
    interp_init(&interp);

    obj_t *l = make_list(0);
    put_env(&interp, NAME("l"), decl(l));

    list_append(l, n_args(1, 42));
    list_append(l, n_args(1, 43));
    TEST_ASSERT_EQUAL(42, list_head(l, NULL)->intval);
    TEST_ASSERT_EQUAL(43, list_head(list_tail(l, NULL), NULL)->intval);

    gc(&interp);
    list_remove_last(l, NULL);

    // List still contains 42.
    TEST_ASSERT_EQUAL(TYPE_LIST, TYPEOF(get_env(&interp, NAME("l"))));
    TEST_ASSERT_EQUAL(42, list_head(l, NULL)->intval);
    TEST_ASSERT_EQUAL(TYPE_NIL, TYPEOF(list_head(list_tail(l, NULL), NULL)));
}

void gc_dict(void) {
    interp_t interp;
    interp_init(&interp);

    obj_t *d1 = dict_obj();
    put_env(&interp, NAME("d1"), decl(d1));

    dict_put(d1, string_obj(NAME("x")), int_obj(42));

    gc(&interp);

    // 'x' still available since d1 still available.
    TEST_ASSERT_EQUAL(TYPE_DICT, TYPEOF(get_env(&interp, NAME("d1"))));

    obj_t *v = dict_get(d1, string_obj(NAME("x")));
    TEST_ASSERT_EQUAL(42, v->intval);
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
    RUN_TEST(gc_list);
    RUN_TEST(gc_dict);
    RUN_TEST(gc_scope);
}

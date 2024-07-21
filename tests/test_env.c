#include "unity/unity.h"
#include "test_env.h"
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/env.h"

#define NAME(x) (c_str_to_bytearray(x))

typedef struct {
    gc_header_t hdr;
    int etc;
} fake_ast_block_t;

static gc_header_t *decl(obj_t *obj) {
    obj->hdr.flags |= F_ENV_DECLARATION;
    return (gc_header_t *) obj;
}

void test_env_init() {
    interp_t interp;
    interp_init(&interp);

    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("pie"))));
}

void test_env_put_get() {
    interp_t interp;
    interp_init(&interp);

    obj_t *not_found = get_env(&interp, NAME("ethel"));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));

    gc_header_t *obj = decl(int_obj(42));

    int error = put_env(&interp, NAME("ethel"), obj);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, error);

    obj_t *found = get_env(&interp, NAME("ethel"));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(found));
    TEST_ASSERT_EQUAL(42, found->intval);
}

void test_env_put_del_get() {
    interp_t interp;
    interp_init(&interp);

    obj_t *not_found = get_env(&interp, NAME("ethel"));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));

    gc_header_t *obj = decl(int_obj(42));

    int error = put_env(&interp, NAME("ethel"), obj);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, error);

    obj_t *found = get_env(&interp, NAME("ethel"));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(found));
    TEST_ASSERT_EQUAL(42, found->intval);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, del_env(&interp, NAME("ethel")));

    not_found = get_env(&interp, NAME("ethel"));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(not_found));
}

void test_env_scopes() {
    interp_t interp;
    interp_init(&interp);
    enter_scope(&interp);

    TEST_ASSERT_EQUAL(1, interp.top);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("one-1"), decl(float_obj(1.1))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("one-2"), decl(float_obj(1.2))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("one-3"), decl(float_obj(1.3))));

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&interp));

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("two-1"), decl(float_obj(2.1))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("two-2"), decl(float_obj(2.2))));

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&interp));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("three-1"), decl(float_obj(3.1))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("three-2"), decl(float_obj(3.2))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("three-3"), decl(float_obj(3.3))));

    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-4"))));
    TEST_ASSERT_EQUAL(3.3, get_env(&interp, NAME("three-3"))->floatval);
    TEST_ASSERT_EQUAL(3.2, get_env(&interp, NAME("three-2"))->floatval);
    TEST_ASSERT_EQUAL(3.1, get_env(&interp, NAME("three-1"))->floatval);
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-3"))));
    TEST_ASSERT_EQUAL(2.2, get_env(&interp, NAME("two-2"))->floatval);
    TEST_ASSERT_EQUAL(2.1, get_env(&interp, NAME("two-1"))->floatval);
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-4"))));
    TEST_ASSERT_EQUAL(1.3, get_env(&interp, NAME("one-3"))->floatval);
    TEST_ASSERT_EQUAL(1.2, get_env(&interp, NAME("one-2"))->floatval);
    TEST_ASSERT_EQUAL(1.1, get_env(&interp, NAME("one-1"))->floatval);

    // Pop the topmost scope.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&interp));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-4"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-1"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-3"))));
    TEST_ASSERT_EQUAL(2.2, get_env(&interp, NAME("two-2"))->floatval);
    TEST_ASSERT_EQUAL(2.1, get_env(&interp, NAME("two-1"))->floatval);
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-4"))));
    TEST_ASSERT_EQUAL(1.3, get_env(&interp, NAME("one-3"))->floatval);
    TEST_ASSERT_EQUAL(1.2, get_env(&interp, NAME("one-2"))->floatval);
    TEST_ASSERT_EQUAL(1.1, get_env(&interp, NAME("one-1"))->floatval);

    // Pop the next scope.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&interp));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-4"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-1"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-1"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-4"))));
    TEST_ASSERT_EQUAL(1.3, get_env(&interp, NAME("one-3"))->floatval);
    TEST_ASSERT_EQUAL(1.2, get_env(&interp, NAME("one-2"))->floatval);
    TEST_ASSERT_EQUAL(1.1, get_env(&interp, NAME("one-1"))->floatval);

    // Pop the last scope. Nothing is in scope.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, leave_scope(&interp));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-4"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("three-1"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("two-1"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-4"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-3"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-2"))));
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("one-1"))));

    // Sanity check. We should be out of all scopes now.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&interp));
    TEST_ASSERT_EQUAL(1, interp.top);

    // And we can push things back in new scopes.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("one-new-1"), decl(int_obj(42))));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, enter_scope(&interp));
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("two-new-1"), decl(int_obj(17))));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&interp, NAME("one-new-1"))));
    TEST_ASSERT_EQUAL(TYPE_INT, TYPEOF(get_env(&interp, NAME("two-new-1"))));
}

void test_env_redefinition_error(void) {
    interp_t interp;
    interp_init(&interp);

    // Not there.
    TEST_ASSERT_EQUAL(TYPE_UNDEF, TYPEOF(get_env(&interp, NAME("thing"))));

    // There.
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, put_env(&interp, NAME("thing"), decl(int_obj(6))));
    TEST_ASSERT_EQUAL(6, get_env(&interp, NAME("thing"))->intval);

    // Can't define it again at this scope.
    TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, put_env(&interp, NAME("thing"), (gc_header_t *) float_obj(1.2)));

    // Also cannot shadow it in a deeper scope.
    enter_scope(&interp);
    TEST_ASSERT_EQUAL(ERR_ENV_SYMBOL_REDEFINED, put_env(&interp, NAME("thing"), (gc_header_t *) float_obj(1.2)));
}


void test_env(void) {
    RUN_TEST(test_env_init);
    RUN_TEST(test_env_put_get);
    RUN_TEST(test_env_put_del_get);
    RUN_TEST(test_env_scopes);
    RUN_TEST(test_env_redefinition_error);
}

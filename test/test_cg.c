#include "unity/unity.h"
#include "test_cg.h"
#include "../inc/cg.h"
#include "../inc/op.h"

void test_cg_init(void) {
    cg_t cg;
    cg_init(&cg);

    TEST_ASSERT_EQUAL(0, cg.max);
    TEST_ASSERT_EQUAL(0, cg.len);
    TEST_ASSERT_EQUAL_PTR(NULL, cg.code);
}

void test_cg_byte(void) {
    cg_t cg;
    cg_init(&cg);

    cg_byte(&cg, VM_OP_NOP);

    TEST_ASSERT_GREATER_OR_EQUAL(1, cg.max);
    TEST_ASSERT_EQUAL(1, cg.len);
    TEST_ASSERT_EQUAL(VM_OP_NOP, cg.code[0]);

    cg_free(&cg);
}

void test_cg_free(void) {
    cg_t cg;
    cg_init(&cg);

    cg_byte(&cg, VM_OP_NOP);
    cg_free(&cg);

    TEST_ASSERT_EQUAL(0, cg.max);
    TEST_ASSERT_EQUAL(0, cg.len);
    TEST_ASSERT_EQUAL_PTR(NULL, cg.code);
}

void test_cg_add_const(void) {
    cg_t cg;
    cg_init(&cg);

    cg_add_const(&cg, int_obj(6), int_obj(42));

    TEST_ASSERT_EQUAL(42, cg_get_const(&cg, int_obj(6))->intval);

    cg_free(&cg);
}

void test_cg(void) {
    RUN_TEST(test_cg_init);
    RUN_TEST(test_cg_byte);
    RUN_TEST(test_cg_free);
    RUN_TEST(test_cg_add_const);
}

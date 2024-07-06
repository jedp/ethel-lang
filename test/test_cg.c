#include "unity/unity.h"
#include "test_cg.h"
#include "../src/comp/cg.h"
#include "../src/comp/map.h"
#include "../src/comp/op.h"

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

    error_t err = ERR_NO_ERROR;

    err |= cg_add_const(&cg, 1, 5);
    err |= cg_add_const(&cg, 2, 6);
    err |= cg_add_const(&cg, 3, 7);
    err |= cg_add_const(&cg, 4, 8);

    TEST_ASSERT_EQUAL(5, cg_get_const(&cg, 1));
    TEST_ASSERT_EQUAL(6, cg_get_const(&cg, 2));
    TEST_ASSERT_EQUAL(7, cg_get_const(&cg, 3));
    TEST_ASSERT_EQUAL(8, cg_get_const(&cg, 4));

    cg_free(&cg);
}

void test_cg(void) {
    RUN_TEST(test_cg_init);
    RUN_TEST(test_cg_byte);
    RUN_TEST(test_cg_free);
    RUN_TEST(test_cg_add_const);
}

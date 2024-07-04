#include "unity/unity.h"
#include "test_cg.h"
#include "../inc/cg.h"

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

    cg_byte(&cg, 0xa5);

    TEST_ASSERT_GREATER_OR_EQUAL(1, cg.max);
    TEST_ASSERT_EQUAL(1, cg.len);
    TEST_ASSERT_EQUAL(0xa5, cg.code[0]);
}

void test_cg_free(void) {
    cg_t cg;
    cg_init(&cg);

    cg_byte(&cg, 0xa5);
    cg_free(&cg);

    TEST_ASSERT_EQUAL(0, cg.max);
    TEST_ASSERT_EQUAL(0, cg.len);
    TEST_ASSERT_EQUAL_PTR(NULL, cg.code);
}

void test_cg(void) {
    RUN_TEST(test_cg_init);
    RUN_TEST(test_cg_byte);
    RUN_TEST(test_cg_free);
}

#include "unity/unity.h"
#include "test_cg.h"
#include "../src/comp/cg.h"
#include "../src/common/op.h"
#include "val.h"

void test_cg_init(void) {
    cg_t cg;
    cg_init(&cg);

    TEST_ASSERT_EQUAL(0, cg.max);
    TEST_ASSERT_EQUAL(0, cg.len);
    TEST_ASSERT_EQUAL_PTR(NULL, cg.bytecode);
}

void test_cg_byte(void) {
    cg_t cg;
    cg_init(&cg);

    cg_byte(&cg, VM_OP_NOP);

    TEST_ASSERT_GREATER_OR_EQUAL(1, cg.max);
    TEST_ASSERT_EQUAL(1, cg.len);
    TEST_ASSERT_EQUAL(VM_OP_NOP, cg.bytecode[0]);

    cg_free(&cg);
}

void test_cg_free(void) {
    cg_t cg;
    cg_init(&cg);

    cg_byte(&cg, VM_OP_NOP);
    cg_free(&cg);

    TEST_ASSERT_EQUAL(0, cg.max);
    TEST_ASSERT_EQUAL(0, cg.len);
    TEST_ASSERT_EQUAL_PTR(NULL, cg.bytecode);
}

void test_cg_add_const(void) {
    cg_t cg;
    cg_init(&cg);

    error_t err = ERR_NO_ERROR;

    uint8_t k1;
    uint8_t k2;
    uint8_t k3;
    uint8_t k4;

    val_t v1 = {.type= VAL_TYPE_INT, .as.intval = 5};
    val_t v2 = {.type= VAL_TYPE_INT, .as.floatval = 6.18f};
    val_t v3 = {.type= VAL_TYPE_INT, .as.charval = 'c'};
    val_t v4 = {.type= VAL_TYPE_INT, .as.byteval = 0xff};

    err |= cg_put_const(&cg, v1, &k1);
    err |= cg_put_const(&cg, v2, &k2);
    err |= cg_put_const(&cg, v3, &k3);
    err |= cg_put_const(&cg, v4, &k4);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
    /*
    TEST_ASSERT_EQUAL(5, cg_get_const(&cg, 1));
    TEST_ASSERT_EQUAL(6, cg_get_const(&cg, 2));
    TEST_ASSERT_EQUAL(7, cg_get_const(&cg, 3));
    TEST_ASSERT_EQUAL(8, cg_get_const(&cg, 4));
     */

    cg_free(&cg);
}

void test_cg(void) {
    RUN_TEST(test_cg_init);
    RUN_TEST(test_cg_byte);
    RUN_TEST(test_cg_free);
    RUN_TEST(test_cg_add_const);
}

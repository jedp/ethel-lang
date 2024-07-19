#include "unity/unity.h"
#include "test_comp.h"
#include "../src/common/op.h"
#include "../src/comp/comp.h"

void test_comp_arithmetic() {
    const char *input = "300 + (1 - -3) * 4 - 8 / 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = comp(input, &cg);
    uint8_t expected[] = {
        VM_OP_LOADI, 1,
        VM_OP_LOADI_1,
        VM_OP_PUSHI, 3,
        VM_OP_NEG,
        VM_OP_SUB,
        VM_OP_PUSHI, 4,
        VM_OP_MUL,
        VM_OP_PUSHI, 8,
        VM_OP_PUSHI, 2,
        VM_OP_DIV,
        VM_OP_SUB,
        VM_OP_ADD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.code, cg.len);

    // Check int const 1.
    map_elem_t v;
    cg_get_const(&cg, 1, &v);
    TEST_ASSERT_EQUAL(300, v.elem.intval);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_boolean_arithmetic() {
    const char *input = "0xff & (37 | 0xa) << 0b00000001";

    cg_t cg;
    cg_init(&cg);
    error_t err = comp(input, &cg);
    uint8_t expected[] = {
        VM_OP_LOADI, 1,
        VM_OP_PUSHI, 37,
        VM_OP_PUSHI, 0xa,
        VM_OP_BIN_OR,
        VM_OP_LOADI_1,
        VM_OP_BIN_SHL,
        VM_OP_BIN_AND,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.code, cg.len);

    // Check int const 1.
    map_elem_t v;
    cg_get_const(&cg, 1, &v);
    TEST_ASSERT_EQUAL(0xff, v.elem.intval);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_assign() {
    // Check right-associativity of assignment operator.
    const char *input = "foo = bar = 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = comp(input, &cg);

    uint8_t expected[] = {
        VM_OP_LOADS, 1,
        VM_OP_LOADS, 2,
        VM_OP_PUSHI, 2,
        VM_OP_ASSIGN,
        VM_OP_ASSIGN,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.code, cg.len);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp() {
    RUN_TEST(test_comp_arithmetic);
    RUN_TEST(test_comp_boolean_arithmetic);
    RUN_TEST(test_comp_assign);
}

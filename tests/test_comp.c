#include "unity/unity.h"
#include "test_comp.h"
#include "../src/common/op.h"
#include "../src/comp/comp.h"
#include "val.h"

void test_comp_error_expect_expression(void) {
    const char *input = "foo = 1 + 2 * 3) / 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    TEST_ASSERT_EQUAL(COMP_ERR_EXPECTED_EXPRESSION, err);
}

void test_comp_error_unexpected_token(void) {
    const char *input = "foo[2 = 'c'";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    TEST_ASSERT_EQUAL(COMP_ERR_UNEXPECTED_TOKEN, err);
}

void test_comp_error_unexpected_token_in_bytearray(void) {
    const char *input = "array { 'a', 'b', 'c', 'd', \"Cookie Monster\" }";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    TEST_ASSERT_EQUAL(COMP_ERR_UNEXPECTED_TOKEN, err);
}

void test_comp_error_non_byte_in_bytearray(void) {
    const char *input = "array { 'a', 'b', 'c', 'd', 256 }";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    TEST_ASSERT_EQUAL(COMP_ERR_NON_BYTE_IN_BYTEARRAY, err);
}

void test_comp_arithmetic(void) {
    const char *input = "300 + (1 - -3) * 4 - 8 / 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);
    uint8_t expected[] = {
        VM_OP_ICONST, 1,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 3,
        VM_OP_NEG,
        VM_OP_SUB,
        VM_OP_IPUSH, 4,
        VM_OP_MUL,
        VM_OP_ADD,
        VM_OP_IPUSH, 8,
        VM_OP_IPUSH, 2,
        VM_OP_DIV,
        VM_OP_SUB,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);

    // Check int const 1.
    val_t v;
    cg_get_const(&cg, 1, &v);
    TEST_ASSERT_EQUAL(300, v.as.intval);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_bit_arithmetic(void) {
    const char *input = "0xff & (37 | 0xa) << 0b00000001";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);
    uint8_t expected[] = {
        VM_OP_ICONST, 1,
        VM_OP_IPUSH, 37,
        VM_OP_IPUSH, 0xa,
        VM_OP_BIN_OR,
        VM_OP_IPUSH_1,
        VM_OP_BIN_SHL,
        VM_OP_BIN_AND,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);

    // Check int const 1.
    val_t v;
    cg_get_const(&cg, 1, &v);
    TEST_ASSERT_EQUAL(0xff, v.as.intval);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_booleans(void) {
    const char *input = "true or false and not true";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_ZPUSH_F,
        VM_OP_ZPUSH_T,
        VM_OP_LOGICAL_NOT,
        VM_OP_LOGICAL_AND,
        VM_OP_LOGICAL_OR,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_nil(void) {
    const char *input = "nil";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_NIL,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_comparators(void) {
    const char *input = "1 < 2 and 3 > 4 or 4 <= 5 and x==6";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_LT,
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_GT,
        VM_OP_LOGICAL_AND,
        VM_OP_IPUSH, 4,
        VM_OP_IPUSH, 5,
        VM_OP_LE,
        VM_OP_SCONST, 1,
        VM_OP_IPUSH, 6,
        VM_OP_EQ,
        VM_OP_LOGICAL_AND,
        VM_OP_LOGICAL_OR,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_assign(void) {
    // Check right-associativity of assignment operator.
    const char *input = "foo = bar = 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_SCONST, 1,
        VM_OP_SCONST, 2,
        VM_OP_IPUSH, 2,
        VM_OP_ASSIGN,
        VM_OP_ASSIGN,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_header(void) {
    // Check magic, major, minor.
    const char *input = "0";

    cg_t cg;
    cg_init(&cg);
    error_t err;

    err = codegen(input, &cg);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);

    uint8_t buf[MIN_BYTECODE_ALLOC] = {0};
    uint32_t size;
    err = compile(&cg, MIN_BYTECODE_ALLOC, buf, &size);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
    uint8_t expected[] = {'E', 'T', 'H', 'L', 0, 1};
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, size - 1);
}

void test_comp_const_pool(void) {
    const char *input = "x = 16909060";

    cg_t cg;
    cg_init(&cg);
    error_t err;

    err = codegen(input, &cg);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);

    uint8_t buf[MIN_BYTECODE_ALLOC] = {0};
    uint32_t size;
    err = compile(&cg, MIN_BYTECODE_ALLOC, buf, &size);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
    uint8_t expected[] = {
        'E', 'T', 'H', 'L', 0, 1,
        2, /* 2 consts */
        CONST_STRING, 1, 'x',
        CONST_INT, 4, 1, 2, 3, 4
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, size - 1);
}

void test_comp_if_statement(void) {
    const char *input = "if (true) then 2 + 2";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 9, 0,
        VM_OP_IPUSH, 2,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_if_else_statements(void) {
    const char *input = "if (true) then 1 else 0";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 8, 0,
        VM_OP_IPUSH_1,
        VM_OP_JMP, 9, 0,
        VM_OP_IPUSH_0,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_if_then_block(void) {
    const char *input = "if (true) then { \n"
                        "    1 + 2        \n"
                        "}";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 8, 0,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_if_then_else_block(void) {
    const char *input = "if (true) then { \n"
                        "    1 + 2        \n"
                        "} else {         \n"
                        "    3 + 4        \n"
                        "}";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 11, 0,
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_JMP, 16, 0,
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_ADD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_if_then_else_if_block(void) {
    const char *input = "if (false) then {       \n"
                        "    1 + 2               \n"
                        "} else if (true) then { \n"
                        "    3 + 4               \n"
                        "} else {                \n"
                        "    5 + 6               \n"
                        "}";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_F,
        VM_OP_JZ, 0x0b, 0,      // if not true, jump to first else.
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_JMP, 0x1c, 0,     // handled true; jump to end.
        VM_OP_ZPUSH_T,          // else
        VM_OP_JZ, 0x17, 0,      // else if not true jump to second else.
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 4,
        VM_OP_ADD,
        VM_OP_JMP, 0x1c, 0,     // handled true; jump to end.
        VM_OP_IPUSH, 5,         // else
        VM_OP_IPUSH, 6,
        VM_OP_ADD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_multiline_block(void) {
    const char *input = "if (true) then { \n"
                        "    x = 1        \n"
                        "    y = 2        \n"
                        "}";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ZPUSH_T,
        VM_OP_JZ, 13, 0,
        VM_OP_SCONST, 1,
        VM_OP_IPUSH_1,
        VM_OP_ASSIGN,
        VM_OP_SCONST, 2,
        VM_OP_IPUSH, 2,
        VM_OP_ASSIGN,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_multiline_parens(void) {
    const char *input = "if (3 > 2 and         \n"
                        "    4 < 5 and         \n"
                        "    5 <= 9) then true   ";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_IPUSH, 3,
        VM_OP_IPUSH, 2,
        VM_OP_GT,
        VM_OP_IPUSH, 4,
        VM_OP_IPUSH, 5,
        VM_OP_LT,
        VM_OP_LOGICAL_AND,
        VM_OP_IPUSH, 5,
        VM_OP_IPUSH, 9,
        VM_OP_LE,
        VM_OP_LOGICAL_AND,
        VM_OP_JZ, 0x15, 0,
        VM_OP_ZPUSH_T,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_subscript(void) {
    const char *input = "\"foo\"[(i + 1) / 2]";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_SCONST, 1,
        VM_OP_SCONST, 2,
        VM_OP_IPUSH_1,
        VM_OP_ADD,
        VM_OP_IPUSH, 2,
        VM_OP_DIV,
        VM_OP_ALOAD,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_bytearray_decl(void) {
    const char *input = "array(20)";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_IPUSH, 20,
        VM_OP_AALLOC,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_bytearray(void) {
    const char *input = "array { '4', 'C', 'C', '!', 1, 2, 3, 4 }";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_ACONST, 1,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_print(void) {
    const char *input = "print(1 + 2)";

    cg_t cg;
    cg_init(&cg);
    error_t err = codegen(input, &cg);

    uint8_t expected[] = {
        VM_OP_IPUSH_1,
        VM_OP_IPUSH, 2,
        VM_OP_ADD,
        VM_OP_PRINT,
        VM_OP_RET,
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, cg.bytecode, cg.len);
    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp(void) {
    RUN_TEST(test_comp_error_expect_expression);
    RUN_TEST(test_comp_error_unexpected_token);
    RUN_TEST(test_comp_error_unexpected_token_in_bytearray);
    RUN_TEST(test_comp_error_non_byte_in_bytearray);
    RUN_TEST(test_comp_arithmetic);
    RUN_TEST(test_comp_bit_arithmetic);
    RUN_TEST(test_comp_booleans);
    RUN_TEST(test_comp_nil);
    RUN_TEST(test_comp_comparators);
    RUN_TEST(test_comp_assign);
    RUN_TEST(test_comp_header);
    RUN_TEST(test_comp_const_pool);
    RUN_TEST(test_comp_if_statement);
    RUN_TEST(test_comp_if_else_statements);
    RUN_TEST(test_comp_if_then_block);
    RUN_TEST(test_comp_if_then_else_block);
    RUN_TEST(test_comp_if_then_else_if_block);
    RUN_TEST(test_comp_multiline_block);
    RUN_TEST(test_comp_multiline_parens);
    RUN_TEST(test_comp_subscript);
    RUN_TEST(test_comp_bytearray_decl);
    RUN_TEST(test_comp_bytearray);
    RUN_TEST(test_comp_print);
}

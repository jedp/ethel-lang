#include "unity/unity.h"
#include "test_comp.h"
#include "../src/comp/comp.h"

void test_comp_arithmetic() {
    const char *input = "300 + (1 - -3) * 4 - 8 / 2";

    error_t err = comp(input);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_boolean_arithmetic() {
    const char *input = "0xff & (37 | 0xa) << 0b00000001";

    error_t err = comp(input);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp_assign() {
    const char *input = "foo = bar = 2";

    error_t err = comp(input);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp() {
    RUN_TEST(test_comp_arithmetic);
    RUN_TEST(test_comp_boolean_arithmetic);
    RUN_TEST(test_comp_assign);
}

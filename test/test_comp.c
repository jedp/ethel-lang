#include "unity/unity.h"
#include "test_comp.h"
#include "../src/comp/comp.h"

void test_comp_arithmetic() {
    const char *input = "300 + -3 * 4 - 12 / 2";

    error_t err = comp(input);

    TEST_ASSERT_EQUAL(ERR_NO_ERROR, err);
}

void test_comp() {
    RUN_TEST(test_comp_arithmetic);
}
#include <stdlib.h>
#include <string.h>
#include "unity/unity.h"
#include "test_ptr.h"
#include "../src/common/ptr.h"

void test_mem_set(void) {
    char *s = malloc(10);

    mem_set(s, 0, 10);
    TEST_ASSERT_EQUAL_STRING("", s);
    TEST_ASSERT_EQUAL(0, strlen(s));

    mem_set(s, (int) 'x', 5);
    TEST_ASSERT_EQUAL_STRING("xxxxx", s);
    TEST_ASSERT_EQUAL(5, strlen(s));
}

void test_ptr(void) {
    RUN_TEST(test_mem_set);
}

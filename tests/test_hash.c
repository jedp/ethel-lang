#include "unity/unity.h"
#include "../common/hash.h"
#include "test_hash.h"

void test_hash_boolean(void) {
    val_t val = {
        .type = VAL_TYPE_BOOL,
        .as.boolval = 1
    };
    TEST_ASSERT_EQUAL(1, val_hash(&val));
}

void test_hash_byte(void) {
    val_t val = {
        .type = VAL_TYPE_BYTE,
        .as.byteval = 42
    };
    TEST_ASSERT_EQUAL(42, val_hash(&val));
}

void test_hash_char(void) {
    val_t val = {
        .type = VAL_TYPE_CHAR,
        .as.charval = 'c'
    };
    TEST_ASSERT_EQUAL(99, val_hash(&val));
}

void test_hash_int(void) {
    val_t val = {
        .type = VAL_TYPE_INT,
        .as.intval = -98765
    };
    TEST_ASSERT_EQUAL(4294868531, val_hash(&val));
}

void test_hash_uint(void) {
    val_t val = {
        .type = VAL_TYPE_UINT,
        .as.uintval =4294967295,
    };
    TEST_ASSERT_EQUAL(4294967295, val_hash(&val));
}

void test_hash_float(void) {
    val_t val = {
        .type = VAL_TYPE_FLOAT,
        .as.floatval = 3.14f
    };
    TEST_ASSERT_EQUAL(3.14, val_hash(&val));
}

void test_hash_obj_str_val(void) {
    obj_str_t *obj_str = obj_str_new("I like pie", 11);
    val_t val = {
        .type = VAL_TYPE_OBJ,
        .as.objval = (obj_t *) obj_str
    };
    TEST_ASSERT_EQUAL(1534350631, val_hash(&val));
}

void test_hash_obj_arr_val(void) {
    uint8_t bytes[] = {1, 2, 3};
    obj_arr_t *obj_arr = obj_arr_new(bytes, 3);
    val_t val = {
        .type = VAL_TYPE_OBJ,
        .as.objval = (obj_t *) obj_arr
    };
    TEST_ASSERT_EQUAL(1456420779, val_hash(&val));
}

void test_hash(void) {
    RUN_TEST(test_hash_boolean);
    RUN_TEST(test_hash_byte);
    RUN_TEST(test_hash_char);
    RUN_TEST(test_hash_int);
    RUN_TEST(test_hash_uint);
    RUN_TEST(test_hash_float);
    RUN_TEST(test_hash_obj_str_val);
    RUN_TEST(test_hash_obj_arr_val);
}

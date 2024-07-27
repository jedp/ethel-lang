#include "unity/unity.h"
#include "../src/common/map.h"
#include "test_map.h"
#include "hash.h"
#include "val.h"

void test_map_new(void) {
    map_t *map = map_new(8, &val_hash, &val_eq);

    TEST_ASSERT_EQUAL(0, map->buckets->nelems);

    map_free(map);
}

void test_map_put(void) {
    map_t *map = map_new(8, &val_hash, &val_eq);
    val_t k = {.type = VAL_TYPE_INT, .as = {1}};
    val_t v = {.type = VAL_TYPE_INT, .as = {42}};

    map_put(map, &k, &v);

    TEST_ASSERT_EQUAL(42, map_get(map, &k)->as.intval);

    map_free(map);
}

void test_map_put_collision(void) {
    map_t *map = map_new(8, &val_hash, &val_eq);
    val_t k1 = {.type = VAL_TYPE_BOOL, .as = {1}};
    val_t v1 = {.type = VAL_TYPE_INT, .as = {1}};
    val_t k2 = {.type = VAL_TYPE_BYTE, .as = {9}};
    val_t v2 = {.type = VAL_TYPE_INT, .as = {2}};
    val_t k3 = {.type = VAL_TYPE_INT, .as = {17}};
    val_t v3 = {.type = VAL_TYPE_INT, .as = {3}};

    map_put(map, &k1, &v1);
    map_put(map, &k2, &v2);
    map_put(map, &k3, &v3);

    TEST_ASSERT_EQUAL(3, map_get(map, &k3)->as.intval);
    TEST_ASSERT_EQUAL(2, map_get(map, &k2)->as.intval);
    TEST_ASSERT_EQUAL(1, map_get(map, &k1)->as.intval);

    map_free(map);
}

void test_map(void) {
    RUN_TEST(test_map_new);
    RUN_TEST(test_map_put);
    RUN_TEST(test_map_put_collision);
}

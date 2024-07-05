#include "unity/unity.h"
#include "../inc/map.h"
#include "../inc/mem.h"
#include "test_map.h"

void test_map_new(void) {
    map_t *map = map_new(8, &hash_primitive, &eq_primitive);

    TEST_ASSERT_EQUAL(0, map->buckets->nelems);

    mem_free(map);
}

void test_map_put(void) {
    map_t *map = map_new(8, &hash_primitive, &eq_primitive);
    map_elem_t k = {.type = MAP_ELEM_INT_TYPE, .elem = 1};
    map_elem_t v = {.type = MAP_ELEM_INT_TYPE, .elem = 42};

    map_put(map, &k, &v);

    TEST_ASSERT_EQUAL(42, map_get(map, &k)->elem.intval);

    mem_free(map);
}

void test_map_put_collision(void) {
    map_t *map = map_new(8, &hash_primitive, &eq_primitive);
    map_elem_t k1 = {.type = MAP_ELEM_BOOL_TYPE, .elem = 1};
    map_elem_t v1 = {.type = MAP_ELEM_INT_TYPE, .elem = 1};
    map_elem_t k2 = {.type = MAP_ELEM_BYTE_TYPE, .elem = 9};
    map_elem_t v2 = {.type = MAP_ELEM_INT_TYPE, .elem = 2};
    map_elem_t k3 = {.type = MAP_ELEM_INT_TYPE, .elem = 17};
    map_elem_t v3 = {.type = MAP_ELEM_INT_TYPE, .elem = 3};

    map_put(map, &k1, &v1);
    map_put(map, &k2, &v2);
    map_put(map, &k3, &v3);

    TEST_ASSERT_EQUAL(3, map_get(map, &k3)->elem.intval);
    TEST_ASSERT_EQUAL(2, map_get(map, &k2)->elem.intval);
    TEST_ASSERT_EQUAL(1, map_get(map, &k1)->elem.intval);

    mem_free(map);
}

void test_map() {
    RUN_TEST(test_map_new);
    RUN_TEST(test_map_put);
    RUN_TEST(test_map_put_collision);
}

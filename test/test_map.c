#include <string.h>
#include "unity/unity.h"
#include "../inc/map.h"
#include "test_map.h"

void test_map_init(void) {
  map_t map;
  map_init(&map, 16);

  TEST_ASSERT_EQUAL(16, map.size);
}

void test_map_put(void) {
  map_t map;
  map_init(&map, 16);

  map_put(&map, "foo", "bar");
}

void test_map_contains(void) {
  map_t map;
  map_init(&map, 16);

  TEST_ASSERT_FALSE(map_contains(&map, "foo"));
  map_put(&map, "foo", "bar");
  TEST_ASSERT_TRUE(map_contains(&map, "foo"));
}

void test_map_update(void) {
  map_t map;
  map_init(&map, 16);

  TEST_ASSERT_FALSE(map_contains(&map, "foo"));
  map_put(&map, "foo", "bar");
  TEST_ASSERT_TRUE(map_contains(&map, "foo"));
  map_put(&map, "foo", "baz");
  TEST_ASSERT_TRUE(map_contains(&map, "foo"));

  TEST_ASSERT_EQUAL(1, map.nelems);
}

void test_map_get(void) {
  map_t map;
  map_init(&map, 16);

  map_put(&map, "foo", "bar");

  char* v;
  TEST_ASSERT_TRUE(map_get(&map, "foo", &v));
  TEST_ASSERT_TRUE(!strcmp("bar", v));
}

void test_map_remove(void) {
  map_t map;
  map_init(&map, 16);

  TEST_ASSERT_FALSE(map_contains(&map, "foo"));
  map_put(&map, "foo", "bar");
  TEST_ASSERT_TRUE(map_contains(&map, "foo"));
  TEST_ASSERT_EQUAL(1, map.nelems);
  map_remove(&map, "foo");
  TEST_ASSERT_FALSE(map_contains(&map, "foo"));
  TEST_ASSERT_EQUAL(0, map.nelems);
}

void test_map_collision_put(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");
  TEST_ASSERT_EQUAL(3, map.nelems);
}

void test_map_collision_contains(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");
  TEST_ASSERT_TRUE(map_contains(&map, "C"));
  TEST_ASSERT_TRUE(map_contains(&map, "c"));
  TEST_ASSERT_TRUE(map_contains(&map, "s"));
  TEST_ASSERT_EQUAL(3, map.nelems);
}

void test_map_collision_update(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");
  map_put(&map, "C", "4");
  map_put(&map, "c", "5");
  map_put(&map, "s", "6");
  TEST_ASSERT_TRUE(map_contains(&map, "C"));
  TEST_ASSERT_TRUE(map_contains(&map, "c"));
  TEST_ASSERT_TRUE(map_contains(&map, "s"));
  TEST_ASSERT_EQUAL(3, map.nelems);
}

void test_map_collision_get(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");

  char* v;
  TEST_ASSERT_TRUE(map_get(&map, "C", &v));
  TEST_ASSERT_TRUE(!strcmp("1", v));
  TEST_ASSERT_TRUE(map_get(&map, "c", &v));
  TEST_ASSERT_TRUE(!strcmp("2", v));
  TEST_ASSERT_TRUE(map_get(&map, "s", &v));
  TEST_ASSERT_TRUE(!strcmp("3", v));
}

void test_map_collision_update_get(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");
  // Update.
  map_put(&map, "C", "4");
  map_put(&map, "c", "5");
  map_put(&map, "s", "6");

  char* v;
  TEST_ASSERT_TRUE(map_get(&map, "C", &v));
  TEST_ASSERT_TRUE(!strcmp("4", v));
  TEST_ASSERT_TRUE(map_get(&map, "c", &v));
  TEST_ASSERT_TRUE(!strcmp("5", v));
  TEST_ASSERT_TRUE(map_get(&map, "s", &v));
  TEST_ASSERT_TRUE(!strcmp("6", v));
}

void test_map_collision_remove(void) {
  map_t map;
  map_init(&map, 16);

  // FNV32(c) % 16 == 2 for each c here.
  map_put(&map, "C", "1");
  map_put(&map, "c", "2");
  map_put(&map, "s", "3");

  TEST_ASSERT_EQUAL(3, map.nelems);
  TEST_ASSERT_TRUE(map_remove(&map, "C"));
  TEST_ASSERT_FALSE(map_contains(&map, "C"));
  TEST_ASSERT_EQUAL(2, map.nelems);
  TEST_ASSERT_TRUE(map_remove(&map, "c"));
  TEST_ASSERT_FALSE(map_contains(&map, "c"));
  TEST_ASSERT_EQUAL(1, map.nelems);
  TEST_ASSERT_TRUE(map_remove(&map, "s"));
  TEST_ASSERT_FALSE(map_contains(&map, "s"));
  TEST_ASSERT_EQUAL(0, map.nelems);
}

void test_map(void) {
  RUN_TEST(test_map_init);
  RUN_TEST(test_map_put);
  RUN_TEST(test_map_contains);
  RUN_TEST(test_map_update);
  RUN_TEST(test_map_get);
  RUN_TEST(test_map_remove);
  RUN_TEST(test_map_collision_put);
  RUN_TEST(test_map_collision_contains);
  RUN_TEST(test_map_collision_update);
  RUN_TEST(test_map_collision_get);
  RUN_TEST(test_map_collision_update_get);
  RUN_TEST(test_map_collision_remove);
}


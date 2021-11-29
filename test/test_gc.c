#include "unity/unity.h"
#include "test_heap.h"
#include "../inc/str.h"
#include "../inc/dict.h"
#include "../inc/heap.h"
#include "../inc/gc.h"

void gc_one_int(void) {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  // Cause scope allocation side effects; then gc and measure heap.
  enter_scope(&env);
  leave_scope(&env);
  gc(&env);
  int init_free = get_heap_info()->bytes_free;

  enter_scope(&env);
  put_env(&env, c_str_to_bytearray("x"), int_obj(42), F_NONE);
  leave_scope(&env);
  gc(&env);

  int final_free = get_heap_info()->bytes_free;
  TEST_ASSERT_EQUAL(init_free, final_free);
  dump_heap();
}

void gc_keep_live_objects(void) {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  obj_t *dict = dict_obj();
  dict_put(dict, byte_obj('a'), int_obj(1));
  dict_put(dict, byte_obj('b'), string_obj(c_str_to_bytearray("ohai")));
  put_env(&env, c_str_to_bytearray("dict"), dict, F_NONE);

  enter_scope(&env);
  put_env(&env, c_str_to_bytearray("nope"), int_obj(99), F_NONE);
  leave_scope(&env);
  gc(&env);

  // "nope" has been deallocated. The other things are still there.
  TEST_ASSERT_EQUAL(TYPE_UNDEF, get_env(&env, c_str_to_bytearray("nope"))->type);
  TEST_ASSERT_EQUAL(1, dict_get(dict, byte_obj('a'))->intval);
  dump_heap();
}

void test_gc(void) {
  RUN_TEST(gc_one_int);
  RUN_TEST(gc_keep_live_objects);
}


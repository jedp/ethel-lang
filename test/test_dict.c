#include "unity/unity.h"
#include "test_dict.h"
#include "../inc/def.h"
#include "../inc/dict.h"
#include "../inc/str.h"
#include "../inc/obj.h"

void test_dict_init(void) {
  obj_t *d = dict_obj();
  TEST_ASSERT_EQUAL(0, d->dict->nelems);
  TEST_ASSERT_EQUAL(16, d->dict->buckets);
}

void test_dict_put(void) {
  obj_t *d = dict_obj();
  obj_t *k = byte_obj('a');
  obj_t *v = int_obj(42);
  dict_put(d, k, v);

  TEST_ASSERT_EQUAL(1, d->dict->nelems);
}

void test_dict_put_collision(void) {
  obj_t *d = dict_obj();
  obj_t *k = byte_obj('a');
  obj_t *v = int_obj(42);
  dict_put(d, k, v);
  TEST_ASSERT_EQUAL(1, d->dict->nelems);

  // 'a' hashes to 97
  // The int 97 will collide.
  k = int_obj(97);
  v = int_obj(43);
  dict_put(d, k, v);
  TEST_ASSERT_EQUAL(2, d->dict->nelems);

  // Sanity check. 'b' hashes to 98; no collision.
  k = byte_obj('b');
  v = int_obj(44);
  dict_put(d, k, v);
  TEST_ASSERT_EQUAL(3, d->dict->nelems);

  // The implementation puts the new nodes at the head of the list.
  TEST_ASSERT_EQUAL(TYPE_INT, d->dict->nodes[1]->dict_nodes->k->type);
  TEST_ASSERT_EQUAL(97, d->dict->nodes[1]->dict_nodes->k->intval);
  TEST_ASSERT_EQUAL(43, d->dict->nodes[1]->dict_nodes->v->intval);
  TEST_ASSERT_EQUAL(TYPE_BYTE, d->dict->nodes[1]->dict_nodes->next->dict_nodes->k->type);
  TEST_ASSERT_EQUAL('a', d->dict->nodes[1]->dict_nodes->next->dict_nodes->k->byteval);
  TEST_ASSERT_EQUAL(42, d->dict->nodes[1]->dict_nodes->next->dict_nodes->v->intval);
  // And the 'b' key. 'b' hashes to 98, which is bucket 2 of 16.
  TEST_ASSERT_EQUAL(TYPE_BYTE, d->dict->nodes[2]->dict_nodes->k->type);
  TEST_ASSERT_EQUAL('b', d->dict->nodes[2]->dict_nodes->k->byteval);
  TEST_ASSERT_EQUAL(44, d->dict->nodes[2]->dict_nodes->v->intval);
}

void test_dict_get(void) {
  obj_t *d = dict_obj();

  obj_t *k = byte_obj('a');
  obj_t *v = int_obj(1);
  dict_put(d, k, v);

  k = int_obj(97); // Collides with 'a'
  v = int_obj(2);
  dict_put(d, k, v);

  k = string_obj(c_str_to_bytearray("foo"));
  v = int_obj(3);
  dict_put(d, k, v);

  k = float_obj(3.14);
  v = int_obj(4);
  dict_put(d, k, v);

  k = boolean_obj(True);
  v = int_obj(5);
  dict_put(d, k, v);

  k = int_obj(1); // Collides with True
  v = int_obj(6);
  dict_put(d, k, v);

  // d['a'] Should be 1, not 2.
  TEST_ASSERT_EQUAL(1, dict_get(d, byte_obj('a'))->intval);
  TEST_ASSERT_EQUAL(2, dict_get(d, int_obj(97))->intval);
  TEST_ASSERT_EQUAL(3, dict_get(d, string_obj(c_str_to_bytearray("foo")))->intval);
  TEST_ASSERT_EQUAL(4, dict_get(d, float_obj(3.14))->intval);
  TEST_ASSERT_EQUAL(5, dict_get(d, boolean_obj(True))->intval);
  TEST_ASSERT_EQUAL(6, dict_get(d, int_obj(1))->intval);

  // Values not present.
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, byte_obj('x'))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, int_obj(-123))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, string_obj(c_str_to_bytearray("bar")))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, float_obj(6.18))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, boolean_obj(False))->type);
}

void test_dict_keys_are_copied_on_put(void) {
  obj_t *d = dict_obj();
  obj_t *k = string_obj(c_str_to_bytearray("moo"));
  obj_t *v = int_obj(42);
  dict_put(d, k, v);

  k->bytearray->data[2] = 'p';

  TEST_ASSERT_EQUAL_STRING("mop", bytearray_to_c_str(k->bytearray));
  TEST_ASSERT_EQUAL(42, dict_get(d, string_obj(c_str_to_bytearray("moo")))->intval);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, string_obj(c_str_to_bytearray("mop")))->type);
}

void test_dict_replace_val(void) {
  obj_t *d = dict_obj();
  dict_put(d, byte_obj('a'), int_obj(1));
  dict_put(d, int_obj(97), int_obj(10)); // Collides with 'a'.
  TEST_ASSERT_EQUAL(1, dict_get(d, byte_obj('a'))->intval);
  TEST_ASSERT_EQUAL(10, dict_get(d, int_obj(97))->intval);

  // Make sure the correct vals are replaced when there's a key collision.

  dict_put(d, byte_obj('a'), int_obj(2));
  TEST_ASSERT_EQUAL(2, dict_get(d, byte_obj('a'))->intval);
  TEST_ASSERT_EQUAL(10, dict_get(d, int_obj(97))->intval);

  dict_put(d, int_obj(97), int_obj(20));
  TEST_ASSERT_EQUAL(2, dict_get(d, byte_obj('a'))->intval);
  TEST_ASSERT_EQUAL(20, dict_get(d, int_obj(97))->intval);
}

void test_dict_remove(void) {
  obj_t *d = dict_obj();
  dict_put(d, byte_obj('a'), int_obj(1));
  dict_put(d, byte_obj('a'), int_obj(3)); // Overwrite just for fun.
  dict_put(d, int_obj(97), int_obj(10)); // Collides with 'a'.
  TEST_ASSERT_EQUAL(3, dict_get(d, byte_obj('a'))->intval);
  TEST_ASSERT_EQUAL(10, dict_get(d, int_obj(97))->intval);

  obj_t *o1 = dict_remove(d, byte_obj('a'));
  TEST_ASSERT_EQUAL(TYPE_INT, o1->type);
  TEST_ASSERT_EQUAL(3, o1->intval);
  TEST_ASSERT_FALSE(dict_contains(d, byte_obj('a')));
  TEST_ASSERT_TRUE(dict_contains(d, int_obj(97)));
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, byte_obj('a'))->type);
  TEST_ASSERT_EQUAL(10, dict_get(d, int_obj(97))->intval);
  TEST_ASSERT_EQUAL(1, d->dict->nelems);

  obj_t *o2 = dict_remove(d, int_obj(97));
  TEST_ASSERT_EQUAL(TYPE_INT, o2->type);
  TEST_ASSERT_EQUAL(10, o2->intval);
  TEST_ASSERT_FALSE(dict_contains(d, byte_obj('a')));
  TEST_ASSERT_FALSE(dict_contains(d, int_obj(97)));
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, byte_obj('a'))->type);
  TEST_ASSERT_EQUAL(TYPE_NIL, dict_get(d, int_obj(97))->type);
  TEST_ASSERT_EQUAL(0, d->dict->nelems);

  // Linked list not broken.
  dict_put(d, byte_obj('a'), int_obj(1));
  TEST_ASSERT_EQUAL(1, dict_get(d, byte_obj('a'))->intval);
}

void test_dict_contains(void) {
  obj_t *d = dict_obj();

  obj_t *k = byte_obj('a');
  obj_t *v = int_obj(1);
  dict_put(d, k, v);

  k = int_obj(97); // Collides with 'a'
  v = int_obj(2);
  dict_put(d, k, v);

  k = string_obj(c_str_to_bytearray("foo"));
  v = int_obj(3);
  dict_put(d, k, v);

  k = float_obj(3.14);
  v = int_obj(4);
  dict_put(d, k, v);

  k = boolean_obj(True);
  v = int_obj(5);
  dict_put(d, k, v);

  k = int_obj(1); // Collides with True
  v = int_obj(6);
  dict_put(d, k, v);

  TEST_ASSERT_EQUAL(True, dict_contains(d, byte_obj('a')));
  TEST_ASSERT_EQUAL(True, dict_contains(d, int_obj(97)));
  TEST_ASSERT_EQUAL(True, dict_contains(d, string_obj(c_str_to_bytearray("foo"))));
  TEST_ASSERT_EQUAL(True, dict_contains(d, float_obj(3.14)));
  TEST_ASSERT_EQUAL(True, dict_contains(d, boolean_obj(True)));
  TEST_ASSERT_EQUAL(True, dict_contains(d, int_obj(1)));

  // Values not present.
  TEST_ASSERT_EQUAL(False, dict_contains(d, byte_obj('x')));
  TEST_ASSERT_EQUAL(False, dict_contains(d, int_obj(-123)));
  TEST_ASSERT_EQUAL(False, dict_contains(d, string_obj(c_str_to_bytearray("bar"))));
  TEST_ASSERT_EQUAL(False, dict_contains(d, float_obj(6.18)));
  TEST_ASSERT_EQUAL(False, dict_contains(d, boolean_obj(False)));
}

void test_dict_resize(void) {
  obj_t *d = dict_obj();
  TEST_ASSERT_EQUAL(DICT_INIT_BUCKETS, d->dict->buckets);

  // Should resize after number of elems exceeds twice number of buckets.
  // More or less.
  obj_t *v = int_obj(0);
  for (int i = 0; i < DICT_INIT_BUCKETS * 2 + 1; i++) {
    dict_put(d, int_obj(i), v);
  }

  TEST_ASSERT_NOT_EQUAL(DICT_INIT_BUCKETS, d->dict->buckets);
}

void test_dict(void) {
  RUN_TEST(test_dict_init);
  RUN_TEST(test_dict_put);
  RUN_TEST(test_dict_put_collision);
  RUN_TEST(test_dict_get);
  RUN_TEST(test_dict_keys_are_copied_on_put);
  RUN_TEST(test_dict_replace_val);
  RUN_TEST(test_dict_remove);
  RUN_TEST(test_dict_contains);
  RUN_TEST(test_dict_resize);
}

#include "unity/unity.h"
#include "util.h"
#include "test_trace.h"
#include "../inc/type.h"
#include "../inc/env.h"
#include "../inc/eval.h"
#include "../inc/obj.h"
#include "../inc/type.h"
#include "../inc/str.h"
#include "../inc/dict.h"

void test_traceable_primitive(void) {
  obj_t *obj = int_obj(42);
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_INT, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
}

void test_traceable_bytearray(void) {
  obj_t *obj = bytearray_obj(0, NULL);
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((gc_header_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((gc_header_t*) obj->bytearray)->flags);
}

void test_traceable_string(void) {
  obj_t *obj = string_obj(c_str_to_bytearray("Hi!"));
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((gc_header_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((gc_header_t*) obj->bytearray)->flags);
}

void test_traceable_list(void) {
  // Use eval because lists are constructed dynamically.
  eval_result_t *result = eval_program("list { 1, 'b', 4.4 }");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_LIST, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);

  obj_list_t *obj_list = obj->list;
  TEST_ASSERT_EQUAL(TYPE_LIST_DATA, ((gc_header_t*) obj_list)->type);

  obj_list_element_t *e1 = obj->list->elems;
  TEST_ASSERT_EQUAL(1, e1->node->intval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((gc_header_t*) e1)->type);
  obj_list_element_t *e2 = e1->next;
  TEST_ASSERT_EQUAL('b', e2->node->byteval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((gc_header_t*) e2)->type);
  obj_list_element_t *e3 = e2->next;
  TEST_ASSERT_EQUAL(4.4, e3->node->floatval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((gc_header_t*) e3)->type);
}

void test_traceable_dict(void) {
  // Use eval because dicts are constructed dynamically.
  eval_result_t *result = eval_program("{ val d = dict  \n"
                                       "  d['x'] = 1.23 \n"
                                       "  d             \n"
                                       "}");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_DICT, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, obj->dict->nelems);

  // Not a very unit-testy way of going, but we have to find the
  // KV bucket that 'x' is in to see if it has the correct tag.
  obj_t *k = byte_obj('x');
  obj_t *hash_obj = get_static_method(TYPEOF(k), METHOD_HASH)(k, NULL);
  uint32_t hv = hash_obj->intval;
  uint32_t bucket_index = hv % obj->dict->buckets;

  dict_node_t *node = obj->dict->nodes[bucket_index];
  TEST_ASSERT_EQUAL(TYPE_DICT_DATA, TYPEOF(node));
}

void test_traceable_function(void) {
  eval_result_t *result = eval_program("fn(x) { x + 1 }");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_FUNCTION, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);

  TEST_ASSERT_EQUAL(TYPE_FUNCTION_PTR_DATA, ((gc_header_t*) obj->func_def)->type);
}

void test_traceable_iterator(void) {
  // Return an iterable and extract its iterator.
  eval_result_t *result = eval_program("1..10");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_RANGE, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);

  static_method get_iterator = get_static_method(TYPEOF(obj), METHOD_ITERATOR);
  TEST_ASSERT_NOT_NULL(get_iterator);
  obj_iter_t *iter = get_iterator(obj, NULL)->iterator;
  TEST_ASSERT_EQUAL(TYPE_ITERATOR_DATA, ((gc_header_t*) iter)->type);
}

void test_traceable_varargs(void) {
  obj_method_args_t *args = wrap_varargs(2,
                                         float_obj(42.0),
                                         int_obj(19));
  TEST_ASSERT_EQUAL(TYPE_VARIABLE_ARGS, ((gc_header_t*) args)->type);
  TEST_ASSERT_EQUAL(TYPE_VARIABLE_ARGS, ((gc_header_t*) args->next)->type);
}

void test_trace(void) {
  RUN_TEST(test_traceable_primitive);
  RUN_TEST(test_traceable_bytearray);
  RUN_TEST(test_traceable_string);
  RUN_TEST(test_traceable_list);
  RUN_TEST(test_traceable_dict);
  RUN_TEST(test_traceable_function);
  RUN_TEST(test_traceable_iterator);
  RUN_TEST(test_traceable_varargs);
}


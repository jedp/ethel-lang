#include <assert.h>
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

void *get_child(void* node, int child_offset) {
  assert(node != NULL);
  assert(child_offset >= 0);
  size_t offset = sizeof(gc_header_t) + (child_offset * sizeof(void*));
  void **child = (void*) (node) + offset;
  return *child;
}

void test_traceable_primitive(void) {
  obj_t *obj = int_obj(42);
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_INT, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(0, hdr->children);
}

void test_traceable_bytearray(void) {
  obj_t *obj = bytearray_obj(0, NULL);
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, TYPEOF((obj_t*) get_child(obj, 0)));
}

void test_traceable_string(void) {
  obj_t *obj = string_obj(c_str_to_bytearray("Hi!"));
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(TYPE_STRING, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, TYPEOF((obj_t*) get_child(obj, 0)));
}

void test_traceable_list(void) {
  // Use eval because lists are constructed dynamically.
  eval_result_t *result = eval_program("list { 1, 'b', 4.4 }");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_LIST, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  // obj->list.
  void* child = (obj_t*) get_child(obj, 0);
  TEST_ASSERT_EQUAL(TYPE_LIST_DATA, TYPEOF((obj_t*) child));

  // list->elems.
  void* grandchild = (obj_t*) get_child(child, 0);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, TYPEOF((obj_t*) grandchild));
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
  TEST_ASSERT_EQUAL(F_ENV_ASSIGNABLE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  void* child = (obj_t*) get_child(obj, 0);
  TEST_ASSERT_EQUAL(TYPE_DICT_DATA, TYPEOF((obj_t*) child));
}

void test_traceable_function(void) {
  eval_result_t *result = eval_program("fn(x) { x + 1 }");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_FUNCTION, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  void* child = (obj_t*) get_child(obj, 0);
  TEST_ASSERT_EQUAL(TYPE_FUNCTION_PTR_DATA, TYPEOF((obj_t*) child));
}

void test_traceable_iterator(void) {
  // Return an iterable and extract its iterator.
  eval_result_t *result = eval_program("1..10");
  obj_t *obj = result->obj;
  gc_header_t *hdr = (gc_header_t*) obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_RANGE, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(1, hdr->children);

  void* child = (obj_t*) get_child(obj, 0);
  TEST_ASSERT_EQUAL(TYPE_RANGE_DATA, TYPEOF((obj_t*) child));
}

void test_traceable_varargs(void) {
  obj_varargs_t *args = wrap_varargs(2,
                                         float_obj(42.0),
                                         int_obj(19));
  gc_header_t *hdr = (gc_header_t*) args;
  TEST_ASSERT_EQUAL(TYPE_VARIABLE_ARGS, hdr->type);
  TEST_ASSERT_EQUAL(F_NONE, hdr->flags);
  TEST_ASSERT_EQUAL(2, hdr->children);

  TEST_ASSERT_EQUAL(TYPE_FLOAT, TYPEOF((obj_t*) get_child(args, 0)));
  TEST_ASSERT_EQUAL(TYPE_VARIABLE_ARGS, TYPEOF((obj_t*) get_child(args, 1)));
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


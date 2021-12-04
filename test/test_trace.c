#include "unity/unity.h"
#include "util.h"
#include "test_trace.h"
#include "../inc/env.h"
#include "../inc/eval.h"
#include "../inc/obj.h"
#include "../inc/str.h"

void test_traceable_primitive(void) {
  obj_t *obj = int_obj(42);
  TEST_ASSERT_EQUAL(TYPE_INT, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);
}

void test_traceable_bytearray(void) {
  obj_t *obj = bytearray_obj(0, NULL);
  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((traceable_obj_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((traceable_obj_t*) obj->bytearray)->flags);
}

void test_traceable_string(void) {
  obj_t *obj = string_obj(c_str_to_bytearray("Hi!"));
  TEST_ASSERT_EQUAL(TYPE_STRING, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);

  TEST_ASSERT_EQUAL(TYPE_BYTEARRAY_DATA, ((traceable_obj_t*) obj->bytearray)->type);
  TEST_ASSERT_EQUAL(F_NONE, ((traceable_obj_t*) obj->bytearray)->flags);
}

void test_traceable_list(void) {
  // Use eval because lists are constructed dynamically.
  eval_result_t *result = eval_program("list { 1, 'b', 4.4 }");
  obj_t *obj = result->obj;
  TEST_ASSERT_EQUAL(ERR_NO_ERROR, result->err);
  TEST_ASSERT_EQUAL(TYPE_LIST, obj->type);
  TEST_ASSERT_EQUAL(F_NONE, obj->flags);

  obj_list_t *obj_list = obj->list;
  TEST_ASSERT_EQUAL(TYPE_LIST_DATA, ((traceable_obj_t*) obj_list)->type);

  obj_list_element_t *e1 = obj->list->elems;
  TEST_ASSERT_EQUAL(1, e1->node->intval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((traceable_obj_t*) e1)->type);
  obj_list_element_t *e2 = e1->next;
  TEST_ASSERT_EQUAL('b', e2->node->byteval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((traceable_obj_t*) e2)->type);
  obj_list_element_t *e3 = e2->next;
  TEST_ASSERT_EQUAL(4.4, e3->node->floatval);
  TEST_ASSERT_EQUAL(TYPE_LIST_ELEM_DATA, ((traceable_obj_t*) e3)->type);
}

void test_trace(void) {
  RUN_TEST(test_traceable_primitive);
  RUN_TEST(test_traceable_bytearray);
  RUN_TEST(test_traceable_string);
  RUN_TEST(test_traceable_list);
}


#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "../inc/type.h"
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/str.h"

obj_t *arg_at(obj_varargs_t *args, int index) {
  int i = 0;
  obj_varargs_t *head = args;
  while(head->arg != NULL) {
    if (i++ == index) return head->arg;

    head = head->next;
    if (i > index) return nil_obj();
  }
  return nil_obj();
}

obj_t *obj_of(type_t type) {
  return (obj_t*) alloc_type(type, F_NONE);
}

obj_t *undef_obj(void) {
  return obj_of(TYPE_UNDEF);
}

obj_t *no_obj(void) {
  return obj_of(TYPE_NOTHING);
}

obj_t *nil_obj(void) {
  return obj_of(TYPE_NIL);
}

obj_t *error_obj(error_t errval) {
  obj_t *obj = obj_of(TYPE_ERROR);
  obj->errval = errval;
  return obj;
}

obj_t *bytearray_obj(size_t size, uint8_t *data) {
  obj_t *obj = obj_of(TYPE_BYTEARRAY);
  if (data) {
    obj->bytearray = bytearray_alloc_with_data(size, data);
  } else {
    obj->bytearray = bytearray_alloc(size);
  }
  return obj;
}

obj_t *int_obj(int i) {
  obj_t *obj = obj_of(TYPE_INT);
  obj->intval = i;
  return obj;
}

obj_t *float_obj(float f) {
  obj_t *obj = obj_of(TYPE_FLOAT);
  obj->floatval = f;
  return obj;
}

obj_t *string_obj(bytearray_t *src) {
  obj_t *obj = obj_of(TYPE_STRING);
  obj->bytearray = bytearray_clone(src);
  return obj;
}

obj_t *byte_obj(byte b) {
  obj_t *obj = obj_of(TYPE_BYTE);
  obj->byteval = b;
  return obj;
}

obj_t *boolean_obj(boolean t) {
  obj_t *obj = obj_of(TYPE_BOOLEAN);
  obj->boolval = t;
  return obj;
}

obj_t *range_obj(int from, int to) {
  obj_t *obj = obj_of(TYPE_RANGE);
  obj->range = (obj_range_t*) alloc_type(TYPE_RANGE_DATA, F_NONE);
  obj->range->from = from;
  obj->range->to = to;
  obj->range->step = 1;
  return obj;
}

obj_t *range_step_obj(int from, int to, int step) {
  if (step < 1) {
    printf("invalid step: %d\n", step);
    return nil_obj();
  }
  obj_t *obj = obj_of(TYPE_RANGE);
  obj->range = (obj_range_t*) alloc_type(TYPE_RANGE_DATA, F_NONE);
  obj->range->from = from;
  obj->range->to = to;
  obj->range->step = step;
  return obj;
}

obj_t *list_obj(obj_list_element_t *elems) {
  obj_t *obj = obj_of(TYPE_LIST);
  obj_list_t *list = (obj_list_t*) alloc_type(TYPE_LIST_DATA, F_NONE);

  list->elems = elems;

  obj->list = list;
  return obj;
}

obj_t *dict_obj(void) {
  obj_t *obj = obj_of(TYPE_DICT);
  ((gc_header_t*) obj)->flags = F_ENV_ASSIGNABLE;
  obj->dict = (obj_dict_t*) alloc_type(TYPE_DICT_DATA, F_NONE);
  if (obj->dict == NULL ||
      dict_init(obj, DICT_INIT_BUCKETS) != ERR_NO_ERROR) {
    mem_free(obj);
    obj = NULL;
    return nil_obj();
  }
  return obj;
}

obj_t *func_obj(void* code, void* scope) {
  obj_t *obj = obj_of(TYPE_FUNCTION);
  obj->func_def = (obj_func_def_t*) alloc_type(TYPE_FUNCTION_PTR_DATA, F_NONE);
  obj->func_def->code = code;
  obj->func_def->scope = scope;

  return obj;
}

obj_t *iterator_obj(obj_t *obj, obj_t *state_obj, obj_t *(*next)(obj_iter_t *iterable)) {
  obj_t *iter = obj_of(TYPE_ITERATOR);
  // Assume the caller has marked obj and state_obj as traceble.
  iter->iterator = (obj_iter_t*) alloc_type(TYPE_ITERATOR_DATA, F_NONE);

  iter->iterator->state = ITER_NOT_STARTED;
  iter->iterator->obj = obj;
  iter->iterator->state_obj = state_obj;
  iter->iterator->next = next;

  return iter;
}

obj_t *return_val(obj_t *val) {
  obj_t *obj = obj_of(TYPE_RETURN_VAL);
  obj->return_val = val;
  return obj;
}

obj_t *break_obj(void) {
  return obj_of(TYPE_BREAK);
}

obj_t *continue_obj(void) {
  return obj_of(TYPE_CONTINUE);
}

obj_varargs_t *wrap_varargs(int n_args, ...) {
  va_list vargs;
  va_start(vargs, n_args);

  obj_varargs_t *args = (obj_varargs_t*) alloc_type(
      TYPE_VARIABLE_ARGS, F_NONE);
  obj_varargs_t *root = args;

  for (int i = 0; i < n_args; i++) {
    obj_t *val = va_arg(vargs, obj_t*);
    args->arg = val;

    if (i < n_args - 1) {
      args->next = (obj_varargs_t*) alloc_type(TYPE_VARIABLE_ARGS, F_NONE);
    } else {
      args->next = NULL;
    }

    args = args->next;
  }

  return root;
}

boolean obj_prim_eq(obj_t *a, obj_t *b) {
  if (TYPEOF(a) != TYPEOF(b)) return False;

  switch (TYPEOF(a)) {
    case TYPE_BOOLEAN:
      return a->boolval == b->boolval;
    case TYPE_INT:
      return a->intval == b->intval;
    case TYPE_FLOAT:
      return a->floatval == b->floatval;
    case TYPE_BYTE:
      return a->byteval == b->byteval;
    default:
      return False;
  }
}


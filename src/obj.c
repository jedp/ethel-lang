#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/str.h"

obj_t *arg_at(obj_t *args_obj, int index) {
  int i = 0;
  obj_method_args_t *args = args_obj->method_args;
  obj_method_args_t *head = args;
  while(head->arg != NULL) {
    if (i++ == index) return head->arg;

    head = head->next;
    if (i > index) return nil_obj();
  }
  return nil_obj();
}

obj_t *obj_of(obj_type_t type) {
  obj_t *obj = mem_alloc(sizeof(obj_t));
  obj->type = type;
  obj->flags = F_NONE;

  return obj;
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

obj_t *bytearray_obj(dim_t size, uint8_t *data) {
  obj_t *obj = obj_of(TYPE_BYTEARRAY);
  bytearray_t *a = mem_alloc(size + 4);
  a->size = size;
  if (data) {
    mem_cp(a->data, data, size);
  } else {
    mem_set(a->data, '\0', size);
  }
  obj->bytearray = a;
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
  bytearray_t *a = mem_alloc(src->size + 4);
  a->size = src->size;
  mem_cp(a->data, src->data, src->size);
  obj->bytearray = a;
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
  obj->range = (range_t) { from, to, 1 };
  return obj;
}

obj_t *range_step_obj(int from, int to, int step) {
  if (step < 1) {
    printf("invalid step: %d\n", step);
    return nil_obj();
  }
  obj_t *obj = obj_of(TYPE_RANGE);
  obj->range = (range_t) { from, to, step };
  return obj;
}

obj_t *list_obj(obj_list_element_t *elems) {
  obj_t *obj = obj_of(TYPE_LIST);
  obj_list_t *list = mem_alloc(sizeof(obj_list_t));

  list->elems = mem_alloc(sizeof(obj_list_element_t));
  list->elems = elems;

  obj->list = list;
  return obj;
}

obj_t *dict_kv_obj(obj_t *k, obj_t *v) {
  obj_t *obj = obj_of(TYPE_DICT_KV);
  obj->dict_nodes = mem_alloc(sizeof(obj_dict_kv_node_t));
  obj->dict_nodes->k = k;
  obj->dict_nodes->v = v;
  obj->dict_nodes->next = NULL;
  return obj;
}

obj_t *dict_obj(void) {
  obj_t *obj = obj_of(TYPE_DICT);
  obj_dict_t *dict = mem_alloc(sizeof(obj_dict_t));
  obj->dict = dict;
  if (obj->dict == NULL ||
      dict_init(obj, DICT_INIT_BUCKETS) != ERR_NO_ERROR) {
    mem_free(obj);
    obj = NULL;
    return nil_obj();
  }
  return obj;
}

obj_t *func_obj(void* code, void* scope) {
  obj_t *obj = obj_of(TYPE_FUNC_PTR);
  obj->func_def = mem_alloc(sizeof(obj_func_def_t));
  obj->func_def->code = code;
  obj->func_def->scope = scope;
  return obj;
}

obj_t *iterator_obj(obj_t *obj, obj_t *state_obj, obj_t *(*next)(obj_iter_t *iterable)) {
  obj_t *iter = obj_of(TYPE_ITERATOR);
  iter->iterator = mem_alloc(sizeof(obj_iter_t));

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

obj_t *method_args_obj(obj_method_args_t *method_args) {
  obj_t *obj = obj_of(TYPE_METHOD_ARGS);
  obj->method_args = mem_alloc(sizeof(obj_method_args_t));
  obj->method_args = method_args;
  return obj;
}

obj_t *wrap_varargs(int n_args, ...) {
  va_list vargs;
  va_start(vargs, n_args);

  obj_method_args_t *args = mem_alloc(sizeof(obj_method_args_t));
  obj_method_args_t *root = args;

  for (int i = 0; i < n_args; i++) {
    obj_t *val = va_arg(vargs, obj_t*);
    args->arg = val;

    if (i < n_args - 1) {
      args->next = mem_alloc(sizeof(obj_method_args_t));
    } else {
      args->next = NULL;
    }

    args = args->next;
  }

  obj_t *obj = obj_of(TYPE_METHOD_ARGS);
  obj->method_args = root;

  return obj;
}

boolean obj_prim_eq(obj_t *a, obj_t *b) {
  if (a->type != b->type) return False;

  switch (a->type) {
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


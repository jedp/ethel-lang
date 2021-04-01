#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/list.h"
#include "../inc/str.h"

obj_t *arg_at(obj_method_args_t *args, int index) {
  int i = 0;
  obj_method_args_t *head = args;
  while(head->arg != NULL) {
    if (i++ == index) return head->arg;

    head = head->next;
    if (i > index) return nil_obj();
  }
  return nil_obj();
}

obj_t *obj_of(obj_type_t type) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = type;
  obj->flags = F_NONE;
  obj->methods = NULL;

  return obj;
}

obj_t *undef_obj() {
  return obj_of(TYPE_UNDEF);
}

obj_t *no_obj() {
  return obj_of(TYPE_NOTHING);
}

obj_t *nil_obj() {
  return obj_of(TYPE_NIL);
}

obj_t *error_obj(error_t errno) {
  obj_t *obj = obj_of(TYPE_ERROR);
  obj->errno = errno;
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

obj_t *string_obj(const char* s) {
  obj_t *obj = obj_of(TYPE_STRING);
  obj->stringval = malloc(strlen(s) + 1);
  strcpy(obj->stringval, s);
  return obj;
}

obj_t *char_obj(char c) {
  obj_t *obj = obj_of(TYPE_CHAR);
  obj->charval = c;
  return obj;
}

obj_t *boolean_obj(boolean t) {
  obj_t *obj = obj_of(TYPE_BOOLEAN);
  obj->boolval = t;
  return obj;
}

obj_t *range_obj(int from, int to) {
  obj_t *obj = obj_of(TYPE_RANGE);
  obj->range = (range_t) { from, to };
  return obj;
}

obj_t *list_obj(char* name, obj_list_element_t *elems) {
  obj_t *obj = obj_of(TYPE_LIST);
  obj_list_t *list = malloc(sizeof(obj_list_t));

  list->type_name = malloc(strlen(name) + 1);
  strcpy(list->type_name, name);

  list->elems = malloc(sizeof(obj_list_element_t));
  list->elems = elems;

  obj->list = list;
  return obj;
}

boolean truthy(obj_t *obj) {
  switch(obj->type) {
    case TYPE_NIL: return False;
    case TYPE_INT: return obj->intval != 0;
    case TYPE_FLOAT: return obj->floatval != 0;
    case TYPE_STRING: return strlen(obj->stringval) > 0;
    case TYPE_CHAR: return obj->charval != 0x0;
    case TYPE_BOOLEAN: return obj->boolval;
    default:
      printf("Unknown type: %d\n", obj->type);
      return False;
      break;
  }
}


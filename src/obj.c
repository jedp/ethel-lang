#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../inc/obj.h"

const char* METHOD_NAME_LENGTH = "length";

obj_t *str_len(obj_t *string_obj, obj_method_args_t /* ignored */ *args) {
  return int_obj(strlen(string_obj->stringval));
}

void make_string_methods(obj_t *obj) {
  obj_method_t *method = malloc(sizeof(obj_method_t));
  method->name = METHOD_NAME_LENGTH;
  method->callable = str_len;
  method->next = NULL;

  obj->methods = method;
}

obj_t *obj_of(obj_type_t type) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = type;
  obj->flags = F_NONE;
  obj->methods = NULL;

  if (obj->type == TYPE_STRING) {
    make_string_methods(obj);
  }
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

obj_t *boolean_obj(bool t) {
  obj_t *obj = obj_of(TYPE_BOOLEAN);
  obj->intval = t ? 1 : 0;
  return obj;
}

obj_t *range_obj(int from, int to) {
  obj_t *obj = obj_of(TYPE_RANGE);
  obj->range = (range_t) { from, to };
  return obj;
}

obj_t *list_obj(char* name, void* es) {
  obj_t *obj = obj_of(TYPE_LIST);
  obj->list.type_name = malloc(strlen(name) + 1);
  strcpy(obj->list.type_name, name);
  obj->list.es = es;
  return obj;
}

bool truthy(obj_t *obj) {
  switch(obj->type) {
    case TYPE_NIL: return false;
    case TYPE_INT: return obj->intval != 0;
    case TYPE_FLOAT: return obj->floatval != 0;
    case TYPE_STRING: return strlen(obj->stringval) > 0;
    case TYPE_CHAR: return obj->charval != 0x0;
    case TYPE_BOOLEAN: return obj->intval == 1;
    default:
      printf("Unknown type: %d\n", obj->type);
      return false;
      break;
  }
}


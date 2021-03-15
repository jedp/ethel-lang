#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../inc/obj.h"

obj_t *obj_of(obj_type_t type) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = type;
  obj->flags = F_NONE;
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

obj_t *string_obj(char* s) {
  obj_t *obj = obj_of(TYPE_STRING);
  obj->stringval = s;
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


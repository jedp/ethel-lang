#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../inc/obj.h"

obj_t *nil_obj() {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = TYPE_NIL;
  return obj;
}

obj_t *int_obj(int i) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = TYPE_INT;
  obj->intval = i;
  return obj;
}

obj_t *float_obj(float f) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = TYPE_FLOAT;
  obj->floatval = f;
  return obj;
}

obj_t *string_obj(char* s) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = TYPE_STRING;
  obj->stringval = s;
  return obj;
}

bool truthy(obj_t *obj) {
  switch(obj->type) {
    case TYPE_NIL: return false;
    case TYPE_INT: return obj->intval != 0;
    case TYPE_FLOAT: return obj->floatval != 0;
    case TYPE_STRING: return strlen(obj->stringval) > 0;
    default:
      printf("Unknown type: %d\n", obj->type);
      return false;
      break;
  }
}


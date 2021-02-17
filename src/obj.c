#include <stdlib.h>
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


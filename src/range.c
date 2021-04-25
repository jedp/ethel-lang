#include <stdio.h>
#include "../inc/range.h"

obj_t *range_contains(obj_t *range_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT) {
    return boolean_obj(False);
  }

  int val = arg->intval;
  return boolean_obj(val >= range_obj->range.from && val <= range_obj->range.to);
}

obj_t *range_subscript(obj_t *range_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to subscript()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT) {
    printf("Int subscript required.\n");
    return nil_obj();
  }

  int val = arg->intval;
  if (val < range_obj->range.from || val > range_obj->range.to) {
    printf("Subscript out of range.\n");
    return nil_obj();
  }

  return int_obj(range_obj->range.from + val);
}

static_method get_range_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_CONTAINS: return range_contains;
    default: return NULL;
  }
}


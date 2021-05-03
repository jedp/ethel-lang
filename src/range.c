#include <stdio.h>
#include "../inc/range.h"
#include "../inc/rand.h"

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

obj_t *range_get(obj_t *range_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
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

obj_t *range_random_choice(obj_t *range_obj, obj_method_args_t *args) {
  uint32_t max = range_obj->range.to - range_obj->range.from;
  // Overflow much?
  return int_obj((rand32() % max) + range_obj->range.from);
}

static_method get_range_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_CONTAINS: return range_contains;
    case METHOD_GET: return range_get;
    case METHOD_RANDOM_CHOICE: return range_random_choice;
    default: return NULL;
  }
}


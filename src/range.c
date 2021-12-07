#include <stdio.h>
#include <limits.h>
#include "../inc/type.h"
#include "../inc/range.h"
#include "../inc/rand.h"
#include "../inc/math.h"
#include "../inc/str.h"
#include "../inc/int.h"
#include "../inc/err.h"

/* Return 1 for upto ranges, -1 for downto. */
static int incr(obj_t *obj) {
  if (obj->range.from < obj->range.to) {
    return 1;
  }
  return -1;
}

static int _range_length(obj_t *obj) {
  return abs(obj->range.from - obj->range.to) + 1;
}

static int _range_get(obj_t *obj, int i, error_t *err) {
  int start = obj->range.from;
  int len = _range_length(obj);

  if (i < 0 || i > len - 1) {
    *err = ERR_RANGE_ERROR;
    return INT_MIN;
  }

  return start + (i * incr(obj));
}

obj_t *range_to_string(obj_t *obj, obj_method_args_t *args) {
  // Don't look.
  obj_t *a = string_obj(c_str_to_bytearray("<Range "));
  obj_t *b = str_add(a, wrap_varargs(1, int_to_string(int_obj(obj->range.from), NULL)));
  obj_t *c = str_add(b, wrap_varargs(1, string_obj(c_str_to_bytearray(".."))));
  obj_t *d = str_add(c, wrap_varargs(1, int_to_string(int_obj(obj->range.to), NULL)));
  obj_t *e = str_add(d, wrap_varargs(1, string_obj(c_str_to_bytearray(">"))));
  return e;
}

obj_t *range_length(obj_t *obj, obj_method_args_t *args) {
  return int_obj(_range_length(obj));
}

obj_t *range_contains(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  if (TYPEOF(arg) != TYPE_INT) {
    return boolean_obj(False);
  }

  int val = arg->intval;
  if (incr(obj) == 1) {
    return boolean_obj(val >= obj->range.from && val <= obj->range.to);
  }
  return boolean_obj(val <= obj->range.from && val >= obj->range.to);
}

obj_t *range_get(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  if (TYPEOF(arg) != TYPE_INT) {
    printf("Int subscript required.\n");
    return nil_obj();
  }

  error_t err = ERR_NO_ERROR;
  int n = _range_get(obj, arg->intval, &err);

  if (err != ERR_NO_ERROR) {
    return nil_obj();
  }

  return int_obj(n);
}

obj_t *range_random_choice(obj_t *obj, obj_method_args_t *args) {
  uint32_t max = obj->range.to - obj->range.from;
  // Overflow much?
  return int_obj((rand32() % max) + obj->range.from);
}

static obj_t *iter_next(obj_iter_t *iterable) {
  int current_val;
  error_t error = ERR_NO_ERROR;

  switch(iterable->state) {
    // The state object stores our offset from the start of the range.
    // So on initialization, set that to 0.
    // Then fall through to the Iterating state.
    case ITER_NOT_STARTED:
      iterable->state = ITER_ITERATING;
      current_val = 0;
      iterable->state_obj->intval = current_val;
      // Fall through to ITERATING.

    // While iterating, get the range element at the state value's offset.
    // Update the offset for the next iteration and return the value.
    case ITER_ITERATING:
      current_val = iterable->state_obj->intval;
      int next_val = _range_get(iterable->obj, current_val, &error);
      if (error != ERR_NO_ERROR) {
        iterable->state = ITER_STOPPED;
        return nil_obj();
      }
      iterable->state_obj->intval += iterable->obj->range.step;
      return int_obj(next_val);

    // Having iterated over all the elements, return Nil as a sentinel.
    case ITER_STOPPED:
      return nil_obj();
    default:
      printf("Unexpected iteration state enum! %d\n", iterable->state);
      return nil_obj();
  }
}

obj_t *range_iterator(obj_t *obj, obj_method_args_t *args) {
  obj_t *start_state = int_obj(0);
  return iterator_obj(obj, start_state, iter_next);
}

static_method get_range_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_TO_STRING: return range_to_string;
    case METHOD_LENGTH: return range_length;
    case METHOD_CONTAINS: return range_contains;
    case METHOD_GET: return range_get;
    case METHOD_RANDOM_CHOICE: return range_random_choice;
    case METHOD_ITERATOR: return range_iterator;
    default: return NULL;
  }
}


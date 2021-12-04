#ifndef __FLOAT_H
#define __FLOAT_H

#include "obj.h"

/* Return the integer hash of the float value. */
obj_t *float_hash(obj_t *obj, obj_t *args_obj);
obj_t *float_copy(obj_t *obj, obj_t *args_obj);
obj_t *float_to_int(obj_t *obj, obj_t *args_obj);
obj_t *float_to_string(obj_t *obj, obj_t *args_obj);
obj_t *float_to_byte(obj_t *obj, obj_t *args_obj);
obj_t *float_to_float(obj_t *obj, obj_t *args_obj);
obj_t *float_abs(obj_t *obj, obj_t *args_obj);
obj_t *float_neg(obj_t *obj, obj_t *args_obj);

/*
 * Return true if the obj and first arg are numerically equal.
 *
 * If arg is not int, byte, or float, always return false.
 */
obj_t *float_eq(obj_t *obj, obj_t *args_obj);
obj_t *float_ne(obj_t *obj, obj_t *args_obj);
obj_t *float_lt(obj_t *obj, obj_t *args_obj);
obj_t *float_gt(obj_t *obj, obj_t *args_obj);
obj_t *float_le(obj_t *obj, obj_t *args_obj);
obj_t *float_ge(obj_t *obj, obj_t *args_obj);
obj_t *float_as(obj_t *obj, obj_t *args_obj);
obj_t *float_math(obj_t *obj, obj_t *args_obj, static_method_ident_t method_id);
obj_t *float_add(obj_t *obj, obj_t *args_obj);
obj_t *float_sub(obj_t *obj, obj_t *args_obj);
obj_t *float_mul(obj_t *obj, obj_t *args_obj);
obj_t *float_div(obj_t *obj, obj_t *args_obj);
obj_t *float_mod(obj_t *obj, obj_t *args_obj);

static_method get_float_static_method(static_method_ident_t method_id);

#endif


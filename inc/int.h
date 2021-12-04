#ifndef __INT_H
#define __INT_H

#include "obj.h"

/* Return the integer hash of the integer. */
obj_t *int_hash(obj_t *obj, obj_method_args_t *args);
obj_t *int_copy(obj_t *obj, obj_method_args_t *args);
obj_t *int_to_int(obj_t *obj, obj_method_args_t *args);
obj_t *int_to_string(obj_t *obj, obj_method_args_t *args);
obj_t *int_to_byte(obj_t *obj, obj_method_args_t *args);
obj_t *int_to_float(obj_t *obj, obj_method_args_t *args);

/*
 * Return true if the obj and first arg are numerically equal.
 *
 * If arg is not int, byte, or float, always return false.
 *
 * Comparison between int and byte is always unsigned.
 */
obj_t *int_eq(obj_t *obj, obj_method_args_t *args);
obj_t *int_ne(obj_t *obj, obj_method_args_t *args);
obj_t *int_lt(obj_t *obj, obj_method_args_t *args);
obj_t *int_gt(obj_t *obj, obj_method_args_t *args);
obj_t *int_le(obj_t *obj, obj_method_args_t *args);
obj_t *int_ge(obj_t *obj, obj_method_args_t *args);
obj_t *int_add(obj_t *obj, obj_method_args_t *args);
obj_t *int_sub(obj_t *obj, obj_method_args_t *args);
obj_t *int_as(obj_t *obj, obj_method_args_t *args);
obj_t *int_abs(obj_t *obj, obj_method_args_t *args);
obj_t *int_neg(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_or(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_xor(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_and(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_shl(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_shr(obj_t *obj, obj_method_args_t *args);
obj_t *int_bitwise_not(obj_t *obj, obj_method_args_t *args);

static_method get_int_static_method(static_method_ident_t method_id);

#endif


#ifndef __BYTE_H
#define __BYTE_H

#include "def.h"
#include "obj.h"

/* Return the integer hash of the integer. */
obj_t *byte_hash(obj_t *obj, obj_t *args_obj);
obj_t *byte_copy(obj_t *obj, obj_t *args_obj);
obj_t *byte_to_int(obj_t *obj, obj_t *args_obj);
obj_t *byte_to_string(obj_t *obj, obj_t *args_obj);
obj_t *byte_to_float(obj_t *obj, obj_t *args_obj);
obj_t *byte_to_byte(obj_t *obj, obj_t *args_obj);

/*
 * Return true if the obj and first arg are numerically equal.
 *
 * If arg is not int, byte, or float, always return false.
 */
obj_t *byte_eq(obj_t *obj, obj_t *args_obj);
obj_t *byte_ne(obj_t *obj, obj_t *args_obj);
obj_t *byte_lt(obj_t *obj, obj_t *args_obj);
obj_t *byte_gt(obj_t *obj, obj_t *args_obj);
obj_t *byte_le(obj_t *obj, obj_t *args_obj);
obj_t *byte_ge(obj_t *obj, obj_t *args_obj);
obj_t *byte_as(obj_t *obj, obj_t *args_obj);
obj_t *bool_math(obj_t *obj, obj_t *args_obj, static_method_ident_t method_id);
obj_t *byte_add(obj_t *obj, obj_t *args_obj);
obj_t *byte_sub(obj_t *obj, obj_t *args_obj);
obj_t *byte_mul(obj_t *obj, obj_t *args_obj);
obj_t *byte_div(obj_t *obj, obj_t *args_obj);
obj_t *byte_mod(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_or(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_xor(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_and(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_shl(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_shr(obj_t *obj, obj_t *args_obj);
obj_t *byte_bitwise_not(obj_t *obj, obj_t *args_obj);

static_method get_byte_static_method(static_method_ident_t method_id);

#endif


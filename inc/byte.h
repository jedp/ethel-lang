#ifndef __BYTE_H
#define __BYTE_H

#include "def.h"
#include "obj.h"

/* Return the integer hash of the integer. */
obj_t *byte_hash(obj_t *obj, obj_method_args_t *args);

/*
 * Return true if the obj and first arg are numerically equal.
 *
 * If arg is not int, byte, or float, always return false.
 */
obj_t *byte_eq(obj_t *obj, obj_method_args_t *args);
obj_t *byte_ne(obj_t *obj, obj_method_args_t *args);
obj_t *byte_lt(obj_t *obj, obj_method_args_t *args);
obj_t *byte_gt(obj_t *obj, obj_method_args_t *args);
obj_t *byte_le(obj_t *obj, obj_method_args_t *args);
obj_t *byte_ge(obj_t *obj, obj_method_args_t *args);

static_method get_byte_static_method(static_method_ident_t method_id);

#endif


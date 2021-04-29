#ifndef __INT_H
#define __INT_H

#include "obj.h"

/* Return the integer hash of the integer. */
obj_t *int_hash(obj_t *obj, obj_method_args_t *args);

/*
 * Return true if the obj and first arg are numerically equal. 
 *
 * If arg is not int, byte, or float, always return false.
 */
obj_t *int_eq(obj_t *obj, obj_method_args_t *args);

/* Return true if the obj and first arg are not int_eq. */
obj_t *int_ne(obj_t *obj, obj_method_args_t *args);

static_method get_int_static_method(static_method_ident_t method_id);

#endif


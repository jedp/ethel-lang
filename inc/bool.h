#ifndef __BOOL_H
#define __BOOL_H

#include "obj.h"

/* Return the 32-bit hash of the boolean, which will just be 0 or 1. */
obj_t *bool_hash(obj_t *obj, obj_method_args_t *args);

/* Return true if both objects are truthy. False otherwise.  */
obj_t *bool_eq(obj_t *obj, obj_method_args_t *args);

/* Return true if the two objects are not bool_eq. */
obj_t *bool_ne(obj_t *obj, obj_method_args_t *args);

/* Cast the boolean to the target type in all sorts of loopy ways. */
obj_t *bool_as(obj_t *obj, obj_method_args_t *args);

static_method get_bool_static_method(static_method_ident_t method_id);

#endif


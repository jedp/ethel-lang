#ifndef __ARR_H
#define __ARR_H

#include "obj.h"

/* Return the number of elements in the array object.  */
obj_t *arr_size(obj_t *arr_obj, obj_method_args_t *args);

/* Get the object in the array at the offset given by the first arg. */
obj_t *arr_get(obj_t *arr_obj, obj_method_args_t *args);

/* Set array offset given by first arg to byte in second arg. */
obj_t *arr_set(obj_t *arr_obj, obj_method_args_t *args);

/* Return true if the byte-compatible first arg is in the array. */
obj_t *arr_contains(obj_t *arr_obj, obj_method_args_t *args);

static_method get_arr_static_method(static_method_ident_t method_id);

#endif


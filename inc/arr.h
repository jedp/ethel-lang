#ifndef __ARR_H
#define __ARR_H

#include "obj.h"

/*
 * Return the integer hash of the array object.
 *
 * This is a function of the contents, so if the contents change,
 * so will the hash.
 */
obj_t *arr_hash(obj_t *obj, obj_t *args_obj);
obj_t *arr_copy(obj_t *obj, obj_t *args_obj);

/* Return the number of elements in the array object.  */
obj_t *arr_size(obj_t *obj, obj_t *args_obj);

/* Get the object in the array at the offset given by the first arg. */
obj_t *arr_get(obj_t *obj, obj_t *args_obj);

/* Set array offset given by first arg to byte in second arg. */
obj_t *arr_set(obj_t *obj, obj_t *args_obj);

/* Return true if the byte-compatible first arg is in the array. */
obj_t *arr_contains(obj_t *obj, obj_t *args_obj);

/* Return true if the arrays are element-wise equal. */
obj_t *arr_eq(obj_t *obj, obj_t *args_obj);

/* Return true if the arrays are element-wise not equal. */
obj_t *arr_ne(obj_t *obj, obj_t *args_obj);

/* Return the array slice as a new array. */
obj_t *arr_slice(obj_t *obj, obj_t *args_obj);

/* Return a random element from the array, or nil if size is 0. */
obj_t *arr_random_choice(obj_t *obj, obj_t *args_obj);

/* Return an iterator over the elements in the array. */
obj_t *arr_iterator(obj_t *obj, obj_t *args_obj);

static_method get_arr_static_method(static_method_ident_t method_id);

#endif


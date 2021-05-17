#ifndef __OBJ_LIST_H
#define __OBJ_LIST_H

#include "obj.h"

/*
 * Return the integer hash of the list.
 *
 * This is a function of the hash of each object in the list.
 */
obj_t *list_hash(obj_t *list_obj, obj_method_args_t *args);

obj_t *list_eq(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_ne(obj_t *list_obj, obj_method_args_t *args);

obj_t *list_len(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_get(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_set(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_slice(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_contains(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_subscript_get(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_subscript_set(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_head(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_tail(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_prepend(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_append(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove_first(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove_last(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove_at(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_random_choice(obj_t *list_obj, obj_method_args_t *args);

static_method get_list_static_method(static_method_ident_t method_id);

#endif


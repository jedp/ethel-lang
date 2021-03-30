#ifndef __LIST_H
#define __LIST_H

#include "obj.h"

obj_t *list_len(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_get(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_slice(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_head(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_tail(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_prepend(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_append(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove_head(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove_last(obj_t *list_obj, obj_method_args_t *args);
obj_t *list_remove(obj_t *list_obj, obj_method_args_t *args);

void make_list_methods(obj_t *list_obj);

#endif


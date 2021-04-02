#ifndef __OBJ_STR_H
#define __OBJ_STR_H

#include "obj.h"

/* Return the length of a c string, not including the terminating null.  */
dim_t c_str_len(const char* s);

/* Return true if the c strings are char-wise equal; false otherwise. */
boolean c_str_eq(const char* a, const char* b);

/* Copy c strings. Target string must already be allocated. */
void c_str_cp(char* dst, const char* src);

/*
 * Append a copy of null-terminated string b to null-terminated string a,
 * copying no more than n chars, then add the trailing null.
 */
char* c_str_ncat(char *a, const char *b, dim_t n);

bytearray_t *bytearray_alloc(dim_t size);

char* bytearray_to_c_str(bytearray_t *a);

bytearray_t *c_str_to_bytearray(const char* s);

obj_t *str_len(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args);

static_method get_str_static_method(static_method_ident_t method_id);

#endif


#ifndef __OBJ_STR_H
#define __OBJ_STR_H

#include "def.h"
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

/* Convert the decimal value in 0..15 to a hex character. */
byte hex_char(int n);

/*
 * Convert a hex string to an int.
 *
 * Because this is going into an int32, we can only accommodate 8 chars (nybbles).
 * Extra leading digits will be ignored. Least signficant digits will be parsed.
 */
int hex_to_int(char* s);

/*
 * Convert a binary string to an int.
 *
 * Because this is going into an int32, we can only accommodate 32 chars.
 * Extra leading digits will be ignored. Least significant digits will be parsed.
 */
int bin_to_int(char* s);

/* Convert an int to a binary string. Uses the least number of digits possible. */
bytearray_t *int_to_bin(unsigned int n);

/* Convert an int to a hex string. Uses the least number of digits possible. */
bytearray_t *int_to_hex(unsigned int n);

/* Return a canonical hexdump of the array as a string. */
obj_t *arr_dump(obj_t *arr_obj);

/* Return a decimal, hex, and binary dump of the byte as a string. */
obj_t *byte_dump(obj_t *byte_obj);

/* Return a hex and binary dump of the int as a string. */
obj_t *int32_dump(obj_t *int_obj);

/* Return a binary dump of the float as a string. */
obj_t *float32_dump(obj_t *float_obj);

bytearray_t *bytearray_alloc(dim_t size);

/* Return a new bytearray that is a bytewise copy of the original. */
bytearray_t *bytearray_clone(bytearray_t *src);

/* Return true if the two arrays have the same contents. */
boolean bytearray_eq(bytearray_t *a, bytearray_t *b);

/* Return true if the string and bytearray have the same contents. */
boolean c_str_eq_bytearray(const char *s, bytearray_t *a);

char* bytearray_to_c_str(bytearray_t *a);

bytearray_t *c_str_to_bytearray(const char* s);

/*
 * Return the integer hash of the string object.
 *
 * This is a function of the contents, so if the contents change,
 * so will the hash.
 */
obj_t *str_hash(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_copy(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_to_string(obj_t *str_obj, obj_method_args_t *args);

obj_t *str_get(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_contains(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_len(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_ne(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_substring(obj_t *str_obj, obj_method_args_t *args);
obj_t *str_random_choice(obj_t *str_obj, obj_method_args_t *args);

static_method get_str_static_method(static_method_ident_t method_id);

#endif


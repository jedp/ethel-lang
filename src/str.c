#include <stdio.h>
#include "../inc/obj.h"
#include "../inc/str.h"

dim_t c_str_len(const char* s) {
  dim_t size = 0;
  const unsigned char *p = (const unsigned char *) s;

  while (*p++ != 0) size++;
  return size;
}

boolean c_str_eq(const char* a, const char* b) {
  const unsigned char *p1 = (const unsigned char *) a;
  const unsigned char *p2 = (const unsigned char *) b;

  while(*p1) {
    if (*p1++ != *p2++) return False;
  }
  return (*p1 == *p2) ? True : False;
}

void c_str_cp(char* dst, const char* src) {
  unsigned char *p1 = (unsigned char *) dst;
  const unsigned char *p2 = (const unsigned char *) src;

  while((*p1++ = *p2++) != 0);
}

void c_str_ncp(char* dst, const char* src, dim_t n) {
  unsigned char *p1 = (unsigned char *) dst;
  const unsigned char *p2 = (const unsigned char *) src;
  dim_t copied = 0;

  while((copied++ < n) && (*p1++ = *p2++) != 0);
  // If we only copied n bytes, add the trailing null.
  if (*p1 != 0) *p1 = 0;
}

char* c_str_ncat(char *a, const char *b, dim_t n) {
  char* s = a;

  c_str_ncp(a + c_str_len(a), b, n);

  return s;
}

obj_t *str_len(obj_t *str_obj, obj_method_args_t *args) {
  return int_obj((int) c_str_len(str_obj->stringval));
}

obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args) {
  return boolean_obj(False);
}

static_method get_str_static_method(static_method_ident_t method_id) {
  switch(method_id) {
    case METHOD_LENGTH: return str_len;
    case METHOD_EQUALS: return str_eq;
    default: return NULL;
  }
}


#ifndef __OBJ_H
#define __OBJ_H

#include <inttypes.h>
#include <stdbool.h>
#include "def.h"

typedef uint8_t obj_type_t;
enum obj_type_enum {
  TYPE_UNKNOWN = 0,
  TYPE_NOTHING,
  TYPE_UNDEF,
  TYPE_NIL,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_BOOLEAN,
  TYPE_RANGE,
  TYPE_LIST,
  TYPE_IDENT,
  TYPE_MAX,
};

typedef struct Obj obj_t;

typedef struct MethodArg {
  obj_t *arg;
  struct MethodArg *next;
} obj_method_args_t;

typedef struct Method {
  const char* name;
  struct Obj *(*callable)(struct Obj *obj, struct MethodArg *args);
  struct Method *next;
} obj_method_t;

typedef struct Range {
  int from;
  int to;
} range_t;

typedef struct ObjListElem {
  obj_t *node;
  struct ObjListElem *next;
} obj_list_element_t;


typedef struct ObjList {
  char* type_name;
  obj_list_element_t *elems;
} obj_list_t;

typedef struct Obj {
  uint16_t type;
  uint16_t flags;
  union {
    int intval;
    float floatval;
    char* stringval;
    char charval;
    range_t range;
    obj_list_t *list;
  };
  obj_method_t *methods;
} obj_t;

typedef obj_t* (*static_method)(obj_t *obj, obj_method_args_t *args);

typedef uint8_t static_method_ident_t;
enum static_method_ident_enum {
  METHOD_NONE = 0,
  METHOD_EQUAL,
  METHOD_LENGTH,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_TAIL,
  METHOD_SLICE,
  METHOD_PREPEND,
  METHOD_APPEND,
  METHOD_REMOVE_FIRST,
  METHOD_REMOVE_LAST,
  METHOD_REMOVE_AT,
};

typedef struct {
  static_method_ident_t ident;
  const char* name;
} static_method_name_t;

static const static_method_name_t static_method_names[] = {
  { .ident = METHOD_EQUAL,         .name = "equal" },
  { .ident = METHOD_LENGTH,        .name = "length" },
  { .ident = METHOD_GET,           .name = "get" },
  { .ident = METHOD_HEAD,          .name = "head" },
  { .ident = METHOD_TAIL,          .name = "tail" },
  { .ident = METHOD_SLICE,         .name = "slice" },
  { .ident = METHOD_PREPEND,       .name = "prepend" },
  { .ident = METHOD_APPEND,        .name = "append" },
  { .ident = METHOD_REMOVE_FIRST,  .name = "removeFirst" },
  { .ident = METHOD_REMOVE_LAST,   .name = "removeLast" },
  { .ident = METHOD_REMOVE_AT,     .name = "removeAt" }
};

static const char* obj_type_names[TYPE_MAX] = {
  "Unknown",
  "Nothing",
  "Undefined",
  "Nil",
  "Int",
  "Float",
  "Char",
  "Str",
  "Bool",
  "Range",
  "List",
  "Identifier"
};

typedef struct StaticMethod {
  const char* name;
  struct Obj *(*callable)(obj_t *obj, obj_method_args_t *args);
} obj_static_method_t;

obj_t *undef_obj();
obj_t *nil_obj();
obj_t *no_obj();
obj_t *int_obj(int);
obj_t *float_obj(float);
obj_t *string_obj(const char*);
obj_t *char_obj(char);
obj_t *boolean_obj(bool);
obj_t *range_obj(int, int);
obj_t *list_obj(char* type_name, obj_list_element_t* elems);

bool truthy(obj_t *obj);

#endif


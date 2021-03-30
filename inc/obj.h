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


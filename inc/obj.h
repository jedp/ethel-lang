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
  TYPE_IDENT,
  TYPE_MAX,
};

typedef struct Obj {
  uint8_t type;
  uint8_t flags;
  union {
    int intval;
    float floatval;
    char* stringval;
    char charval;
  };
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

bool truthy(obj_t *obj);

#endif


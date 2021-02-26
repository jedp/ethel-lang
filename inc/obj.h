#ifndef __OBJ_H
#define __OBJ_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum {
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
} obj_type_t;

typedef struct Obj {
  uint8_t type;
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
  "Integer",
  "Float",
  "Char",
  "String",
  "Boolean",
  "Identifier"
};

obj_t *undef_obj();
obj_t *no_obj();
obj_t *nil_obj();
obj_t *int_obj(int);
obj_t *float_obj(float);
obj_t *string_obj(char*);
obj_t *char_obj(char);
obj_t *boolean_obj(bool);

bool truthy(obj_t *obj);

#endif


#ifndef __OBJ_H
#define __OBJ_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum {
  TYPE_UNKNOWN = 0,
  TYPE_NIL,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_IDENT,
  TYPE_MAX,
} obj_type_t;

typedef struct Obj {
  uint8_t type;
  union {
    int intval;
    float floatval;
    char* stringval;
  };
} obj_t;

static const char* obj_type_names[TYPE_MAX] = {
  "Unknown",
  "Nil",
  "Integer",
  "Float",
  "String",
  "Identifier"
};

obj_t *nil_obj();
obj_t *int_obj(int);
obj_t *float_obj(float);
obj_t *string_obj(char*);

bool truthy(obj_t *obj);

#endif


#ifndef __OBJ_H
#define __OBJ_H

#include <inttypes.h>
#include "err.h"
#include "def.h"

typedef uint8_t obj_type_t;
enum obj_type_enum {
  TYPE_UNKNOWN = 0,
  TYPE_NOTHING,
  TYPE_UNDEF,
  TYPE_NIL,
  TYPE_ERROR,
  TYPE_FUNC_PTR,
  TYPE_RETURN_VAL,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BYTE,
  TYPE_BYTEARRAY,
  TYPE_STRING,
  TYPE_BOOLEAN,
  TYPE_RANGE,
  TYPE_LIST,
  TYPE_IDENT,
  TYPE_MAX,
};

static const char* obj_type_names[TYPE_MAX] = {
  "Unknown",
  "Nothing",
  "Undefined",
  "Nil",
  "Error",
  "Function",
  "Return Val",
  "Int",
  "Float",
  "Byte",
  "Byte Array",
  "Str",
  "Bool",
  "Range",
  "List",
  "Identifier"
};

typedef struct Obj obj_t;

typedef struct MethodArg {
  obj_t *arg;
  struct MethodArg *next;
} obj_method_args_t;

typedef struct Method {
  bytearray_t *name;
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
  bytearray_t *type_name;
  obj_list_element_t *elems;
} obj_list_t;

typedef struct ObjFuncArg {
  bytearray_t *name;
  struct ObjFuncArg *next;
} obj_func_arg_t;

typedef struct ObjFuncDef {
  /* Pointer to the ast_func_def_t to execute. */
  void* code;
  /* Pointer to the env_sym_t scope the function was declared in. */
  void* scope;
} obj_func_def_t;

typedef struct Obj {
  uint16_t type;
  uint16_t flags;
  union {
    error_t errno;
    int intval;
    int boolval;
    float floatval;
    byte byteval;
    range_t range;
    obj_list_t *list;
    bytearray_t *bytearray;
    obj_func_def_t *func_def;
    struct Obj *return_val;
  };
  obj_method_t *methods;
} obj_t;

typedef obj_t* (*static_method)(obj_t *obj, obj_method_args_t *args);
typedef obj_t* (*binop_method)(obj_t *obj, obj_t *other);

obj_t *arg_at(obj_method_args_t *args, int index);

typedef uint8_t static_method_ident_t;
enum static_method_ident_enum {
  METHOD_NONE = 0,
  METHOD_HASH,
  METHOD_EQ,
  METHOD_NE,
  METHOD_CMP,
  METHOD_LENGTH,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_TAIL,
  METHOD_SLICE,
  METHOD_SUBSTR,
  METHOD_CONTAINS,
  METHOD_PREPEND,
  METHOD_APPEND,
  METHOD_REMOVE_FIRST,
  METHOD_REMOVE_LAST,
  METHOD_REMOVE_AT,
  METHOD_DUMP,
};

typedef struct {
  static_method_ident_t ident;
  const char* name;
} static_method_name_t;

static const static_method_name_t static_method_names[] = {
  { .ident = METHOD_HASH,          .name = "hash" },
  { .ident = METHOD_EQ,            .name = "equals" },
  { .ident = METHOD_CMP,           .name = "compare" },
  { .ident = METHOD_LENGTH,        .name = "length" },
  { .ident = METHOD_GET,           .name = "get" },
  { .ident = METHOD_HEAD,          .name = "head" },
  { .ident = METHOD_TAIL,          .name = "tail" },
  { .ident = METHOD_SLICE,         .name = "slice" },
  { .ident = METHOD_SUBSTR,        .name = "substring" },
  { .ident = METHOD_CONTAINS,      .name = "contains" },
  { .ident = METHOD_PREPEND,       .name = "prepend" },
  { .ident = METHOD_APPEND,        .name = "append" },
  { .ident = METHOD_REMOVE_FIRST,  .name = "removeFirst" },
  { .ident = METHOD_REMOVE_LAST,   .name = "removeLast" },
  { .ident = METHOD_REMOVE_AT,     .name = "removeAt" },
  { .ident = METHOD_DUMP,          .name = "dump" },
};

obj_t *undef_obj(void);
obj_t *nil_obj(void);
obj_t *no_obj(void);
obj_t *error_obj(error_t errno);
obj_t *int_obj(int);
obj_t *float_obj(float);
obj_t *byte_obj(byte);
obj_t *bytearray_obj(dim_t size, uint8_t *data);
obj_t *string_obj(bytearray_t *src);
obj_t *boolean_obj(boolean);
obj_t *range_obj(int, int);
obj_t *list_obj(bytearray_t *type_name, obj_list_element_t* elems);
obj_t *func_obj(void* code, void* scope);
obj_t *return_val(obj_t *val);
obj_method_args_t *wrap_varargs(int n_args, ...);
boolean obj_prim_eq(obj_t *a, obj_t *b);

boolean truthy(obj_t *obj);

#endif


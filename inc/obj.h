#ifndef __OBJ_H
#define __OBJ_H

#include <inttypes.h>
#include "err.h"
#include "def.h"

enum iter_state {
  ITER_NOT_STARTED,
  ITER_ITERATING,
  ITER_STOPPED,
};

typedef struct Obj obj_t;

typedef struct VariableArg {
  gc_header_t hdr;
  obj_t *arg;
  struct VariableArg *next;
} obj_method_args_t;

typedef struct Range {
  int from;
  int to;
  int step;
} range_t;

typedef struct ObjListElem {
  gc_header_t hdr;
  obj_t *node;
  struct ObjListElem *next;
} obj_list_element_t;

typedef struct ObjList {
  gc_header_t hdr;
  obj_list_element_t *elems;
} obj_list_t;

typedef struct ObjDictKvNode {
  gc_header_t hdr;
  obj_t *k;
  obj_t *v;
  struct ObjDictKvNode *next;
  uint32_t hash_val;
} dict_node_t;

typedef struct ObjDictElem {
  gc_header_t hdr;
  dict_node_t **nodes;
  uint32_t buckets;
  uint32_t nelems;
} obj_dict_t;

typedef struct ObjFuncDef {
  gc_header_t hdr;
  /* Pointer to the ast_func_def_t to execute. */
  void* code;
  /* Pointer to the env_sym_t scope the function was declared in. */
  void* scope;
} obj_func_def_t;

typedef struct ObjIterator {
  gc_header_t hdr;
  obj_t *obj;
  obj_t *state_obj;
  // Don't link to 'next' for GC; It's a pointer to native code.
  struct Obj *(*next)(struct ObjIterator *iterable);
  int state;
} obj_iter_t;

typedef struct Obj {
  gc_header_t hdr;
  union {
    error_t errval;
    int intval;
    int boolval;
    float floatval;
    byte byteval;
    range_t range;
    obj_list_t *list;
    obj_dict_t *dict;
    bytearray_t *bytearray;
    obj_func_def_t *func_def;
    obj_iter_t *iterator;
    struct Obj *return_val;
  };
} obj_t;

typedef obj_t* (*static_method)(obj_t *obj, obj_method_args_t *args);
typedef obj_t* (*binop_method)(obj_t *obj, obj_t *other);
typedef obj_t* (*iterator_next)(obj_t *obj);

obj_t *arg_at(obj_method_args_t *args, int index);

typedef uint8_t static_method_ident_t;
enum static_method_ident_enum {
  METHOD_NONE = 0,
  METHOD_HASH,
  METHOD_COPY,
  METHOD_TO_STRING,
  METHOD_TO_INT,
  METHOD_TO_BYTE,
  METHOD_TO_FLOAT,
  METHOD_ADD,
  METHOD_SUB,
  METHOD_MUL,
  METHOD_DIV,
  METHOD_MOD,
  METHOD_ABS,
  METHOD_RANDOM_CHOICE,
  METHOD_NEG,
  METHOD_EQ,
  METHOD_NE,
  METHOD_LT,
  METHOD_GT,
  METHOD_LE,
  METHOD_GE,
  METHOD_BITWISE_AND,
  METHOD_BITWISE_OR,
  METHOD_BITWISE_XOR,
  METHOD_BITWISE_NOT,
  METHOD_BITWISE_SHL,
  METHOD_BITWISE_SHR,
  METHOD_CAST,
  METHOD_LENGTH,
  METHOD_KEYS,
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
  METHOD_REMOVE_ELEM,
  METHOD_ITERATOR,
  METHOD_DUMP,
};

typedef struct {
  static_method_ident_t ident;
  const char* name;
} static_method_name_t;

static const static_method_name_t static_method_names[] = {
  { .ident = METHOD_NONE,          .name = "<none>" },
  { .ident = METHOD_HASH,          .name = "hash" },
  { .ident = METHOD_COPY,          .name = "copy" },
  { .ident = METHOD_TO_STRING,     .name = "toString" },
  { .ident = METHOD_ABS,           .name = "abs" },
  { .ident = METHOD_RANDOM_CHOICE, .name = "rand" },
  { .ident = METHOD_NEG,           .name = "neg" },
  { .ident = METHOD_EQ,            .name = "eq" },
  { .ident = METHOD_NE,            .name = "ne" },
  { .ident = METHOD_LT,            .name = "lt" },
  { .ident = METHOD_GT,            .name = "gt" },
  { .ident = METHOD_LE,            .name = "le" },
  { .ident = METHOD_GE,            .name = "ge" },
  { .ident = METHOD_BITWISE_AND,   .name = "and" },
  { .ident = METHOD_BITWISE_OR,    .name = "or" },
  { .ident = METHOD_BITWISE_XOR,   .name = "xor" },
  { .ident = METHOD_BITWISE_NOT,   .name = "not" },
  { .ident = METHOD_BITWISE_SHL,   .name = "shl" },
  { .ident = METHOD_BITWISE_SHR,   .name = "shr" },
  { .ident = METHOD_CAST,          .name = "as" },
  { .ident = METHOD_LENGTH,        .name = "length" },
  { .ident = METHOD_KEYS,          .name = "keys" },
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
  { .ident = METHOD_REMOVE_ELEM,   .name = "remove" },
  { .ident = METHOD_DUMP,          .name = "dump" },
};

obj_t *undef_obj(void);
obj_t *nil_obj(void);
obj_t *no_obj(void);
obj_t *error_obj(error_t errval);
obj_t *int_obj(int);
obj_t *float_obj(float);
obj_t *byte_obj(byte);
obj_t *bytearray_obj(size_t size, uint8_t *data);
obj_t *string_obj(bytearray_t *src);
obj_t *boolean_obj(boolean);
obj_t *range_obj(int from_inclusive, int to_inclusive);
obj_t *range_step_obj(int from_inclusive, int to_inclusive, int step);
obj_t *list_obj(obj_list_element_t* elems);
obj_t *dict_obj(void);
obj_t *func_obj(void* code, void* scope);
obj_t *iterator_obj(obj_t *obj, obj_t *state_obj, obj_t *(*next)(obj_iter_t *iterable));
obj_t *return_val(obj_t *val);
obj_t *break_obj(void);
obj_t *continue_obj(void);
obj_method_args_t *wrap_varargs(int n_args, ...);
boolean obj_prim_eq(obj_t *a, obj_t *b);

#endif


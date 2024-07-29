#pragma once

#include <stdbool.h>
#include "vm_err.h"
#include "../comp/cg.h"

#define VM_DATA_STACK_SIZE (64)

#define VALID_SUBSCRIPT_INDEX(elem) \
    ((elem)->type == VM_STACK_BYTE_TYPE || (elem)->type ==VM_STACK_INT_TYPE)

/*
 * Order matters for determining which numeric values can be up-cast
 * with no loss.
 */
typedef enum {
    VM_STACK_ERROR_NO_TYPE,
    VM_STACK_NIL_TYPE,
    VM_STACK_BOOL_TYPE,
    VM_STACK_BYTE_TYPE,
    VM_STACK_INT_TYPE,
    VM_STACK_FLOAT_TYPE,
    VM_STACK_OBJ_TYPE,
    VM_STACK_ADDR_TYPE,
    VM_STACK_ELEM_TYPE_MAX,
} vm_stack_elem_type_t;

static const char *vm_stack_elem_type_names[] = {
    [VM_STACK_ERROR_NO_TYPE] = "No type!",
    [VM_STACK_NIL_TYPE] = "Nil",
    [VM_STACK_BOOL_TYPE] = "Boolean",
    [VM_STACK_BYTE_TYPE] = "Byte",
    [VM_STACK_INT_TYPE] = "Int",
    [VM_STACK_FLOAT_TYPE] = "Float",
    [VM_STACK_OBJ_TYPE] = "Object",
    [VM_STACK_ADDR_TYPE] = "Address",
};

#define TYPE_IS_NUMERIC(t) ((t == VM_STACK_BYTE_TYPE || t == VM_STACK_INT_TYPE || t == VM_STACK_FLOAT_TYPE))

/*
 * Stack elem is tagged union to make straight arithmetic
 * faster with no obj lookup.
 */
typedef struct vm_stack_elem_t {
    vm_stack_elem_type_t type;
    union {
        uint8_t byteval;
        int intval;
        float floatval;
        int boolval;
        obj_t *objval;
        uint32_t addrval;
    } as;
} vm_stack_elem_t;

typedef struct vm_stack_t {
    uint32_t size;
    // top is pointer into buf.
    vm_stack_elem_t *top;
    vm_stack_elem_t buf[VM_DATA_STACK_SIZE];
} vm_stack_t;

typedef struct vm_t {
    uint32_t bytecode_size;
    uint32_t code_start;
    uint8_t *bytecode;
    map_t *consts;
    vm_stack_t *stack;
    // pc is a pointer into the bytecode bytearray.
    uint8_t *pc;
} vm_t;

void runtime_error(vm_t *vm, const char *fmt, ...);

bool runtime_check(vm_t *vm, bool condition, const char *message);

map_err_t vm_map_put(map_t *map, val_t v, uint8_t *k);

map_err_t vm_map_get(map_t *map, uint8_t k, val_t *v);

vm_err_t vm_stack_reset(vm_t *vm);

vm_err_t vm_init(vm_t *vm, const uint8_t *bytes, uint32_t length);

vm_err_t vm_free(vm_t *vm);

vm_err_t vm_exec(vm_t *vm);

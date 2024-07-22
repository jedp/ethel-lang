#pragma once

#include <stdbool.h>
#include "../comp/cg.h"
#include "../common/def.h"
#include "err.h"

#define VM_DATA_STACK_SIZE (64)

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
    VM_STACK_ADDR_TYPE,
    VM_STACK_ELEM_TYPE_MAX,
} vm_stack_elem_type_t;

static const char *vm_stack_elem_type_names[VM_STACK_ELEM_TYPE_MAX] = {
    [VM_STACK_ERROR_NO_TYPE] = "No type!",
    [VM_STACK_NIL_TYPE] = "Nil",
    [VM_STACK_BOOL_TYPE] = "Boolean",
    [VM_STACK_BYTE_TYPE] = "Byte",
    [VM_STACK_INT_TYPE] = "Int",
    [VM_STACK_FLOAT_TYPE] = "Float",
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
        uint8_t addrval;
    };
} vm_stack_elem_t;

typedef struct vm_stack_t {
    uint32_t size;
    // top is pointer into buf.
    vm_stack_elem_t *top;
    vm_stack_elem_t buf[VM_DATA_STACK_SIZE];
} vm_stack_t;

typedef struct vm_t {
    uint32_t bytecode_size;
    // pc is a pointer into the cg->bytecode bytearray.
    uint8_t *pc;
    cg_t *cg;
    vm_stack_t *stack;
} vm_t;

void runtime_error(vm_t *vm, const char *message);

bool runtime_check(vm_t *vm, bool condition, const char *message);

error_t vm_init(vm_t *vm);

error_t vm_free(vm_t *vm);

error_t vm_stack_reset(vm_t *vm);

vm_stack_elem_t vm_stack_elem_new(void);

error_t vm_stack_push(vm_t *vm, vm_stack_elem_t *e);

error_t vm_stack_push_byte(vm_t *vm, uint8_t b);

error_t vm_stack_push_int(vm_t *vm, int i);

error_t vm_stack_push_boolean(vm_t *vm, bool z);

error_t vm_stack_push_nil(vm_t *vm);

vm_stack_elem_t *vm_stack_peek(vm_t *vm);

vm_stack_elem_t *vm_stack_pop(vm_t *vm);

error_t vm_load_code(vm_t *vm, uint8_t *bytes, size_t size);

error_t vm_exec(vm_t *vm);

error_t vm_interp(vm_t *vm, const char *input);

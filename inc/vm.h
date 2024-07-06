#pragma once

#include "cg.h"
#include "def.h"
#include "err.h"

#define VM_DATA_STACK_SIZE (64)

/*
 * Order matters for determining which numeric values can be up-cast
 * with no loss.
 */
typedef enum {
    VM_STACK_ERROR_NO_TYPE,
    VM_STACK_BOOL_TYPE,
    VM_STACK_BYTE_TYPE,
    VM_STACK_INT_TYPE,
    VM_STACK_FLOAT_TYPE,
    VM_STACK_OBJ_TYPE,
} vm_stack_elem_type_t;

/*
 * Stack elem is tagged union to make straight arithmetic
 * faster with no obj lookup.
 */
typedef struct vm_stack_elem_t {
    gc_header_t hdr;
    vm_stack_elem_type_t type;
    union {
        uint8_t byteval;
        int intval;
        float floatval;
        int boolval;
        obj_t *obj_ptr;
    };
} vm_stack_elem_t;

typedef struct vm_stack_t {
    gc_header_t hdr;
    uint32_t size;
    // top is pointer into buf.
    vm_stack_elem_t *top;
    vm_stack_elem_t buf[VM_DATA_STACK_SIZE];
} vm_stack_t;

typedef struct vm_t {
    gc_header_t hdr;
    uint32_t code_size;
    // pc is a pointer into the cg->code bytearray.
    uint8_t *pc;
    cg_t *cg;
    vm_stack_t *stack;
} vm_t;

error_t vm_init(vm_t *vm);

error_t vm_free(vm_t *vm);

error_t vm_stack_reset(vm_t *vm);

vm_stack_elem_t *vm_stack_elem_new(void);

error_t vm_stack_push_int(vm_t *vm, int i);

error_t vm_stack_push(vm_t *vm, vm_stack_elem_t *e);

vm_stack_elem_t *vm_stack_peek(vm_t *vm);

vm_stack_elem_t *vm_stack_pop(vm_t *vm);

error_t vm_load_code(vm_t *vm, bytearray_t *bytes);

error_t vm_interp(vm_t *vm);

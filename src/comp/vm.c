#include "cg.h"
#include "def.h"
#include "dis.h"
#include "err.h"
#include "map.h"
#include "mem.h"
#include "op.h"
#include "vm.h"

#define READ_BYTE() (*vm->pc++)

#define LOADI() (cg_get_const(vm->cg, READ_BYTE()))

static error_t exec(vm_t *vm) {
    for (;;) {
        uint8_t bytecode;
        switch (bytecode = READ_BYTE()) {
            case VM_OP_NOP:
                break;
            case VM_OP_LOADI:
                vm_stack_push_int(vm, LOADI());
                break;
            case VM_OP_RET:
                return ERR_VM_INTERP_OK;
            default:
                return ERR_VM_RUNTIME_ERROR;
        }
    }
}

error_t vm_init(vm_t *vm) {
    cg_t *cg = mem_alloc(sizeof(cg_t));
    if (cg == NULL)
        return ERR_OUT_OF_MEMORY;
    cg_init(cg);

    // A stack of objects. Top is a pointer into buf.
    // Top always points to the next value to be filled.
    // If top == buf, stack is empty.
    vm_stack_t *stack = mem_alloc(sizeof(vm_stack_t));
    if (stack == NULL)
        return ERR_OUT_OF_MEMORY;

    vm_stack_elem_t *stack_buf = mem_alloc(sizeof(vm_stack_elem_t) * VM_DATA_STACK_SIZE);
    stack->size = 0;
    stack->top = stack->buf;

    vm->cg = cg;
    vm->pc = cg->code;
    vm->code_size = vm->cg->len;
    vm->stack = stack;

    return ERR_NO_ERROR;
}

error_t vm_free(vm_t *vm) {
    cg_free(vm->cg);
    mem_free(vm->cg);
    vm_init(vm);

    return ERR_NO_ERROR;
}

error_t vm_stack_reset(vm_t *vm) {
    vm->stack->top = vm->stack->buf;
    return ERR_NO_ERROR;
}

error_t vm_stack_push_int(vm_t *vm, int i) {
    if (vm->stack->top > vm->stack->buf + sizeof(vm_stack_elem_t * ) * VM_DATA_STACK_SIZE) {
        return ERR_VM_STACK_OVERFLOW;
    }
    vm->stack->top->type = VM_STACK_INT_TYPE;
    vm->stack->top->intval = i;
    vm->stack->top += sizeof(vm->stack->top);
    return ERR_NO_ERROR;
}

vm_stack_elem_t *vm_stack_peek(vm_t *vm) {
    if (vm->stack->top == vm->stack->buf) {
        return NULL;
    }
    return vm->stack->top - sizeof(vm->stack->top);
}

vm_stack_elem_t *vm_stack_pop(vm_t *vm) {
    vm->stack->top -= sizeof(vm->stack->top);
    return vm->stack->top;
}

error_t vm_load_code(vm_t *vm, bytearray_t *bytecode) {
    error_t err = vm_free(vm);
    if (err != ERR_NO_ERROR)
        return err;

    vm_init(vm);
    cg_bytes(vm->cg, bytecode);
    vm->pc = vm->cg->code;
    vm->code_size = vm->cg->len;
    return ERR_NO_ERROR;
}

error_t vm_interp(vm_t *vm) {
    printf("Executing code:\n");
    print_dis(vm->cg);
    return exec(vm);
}

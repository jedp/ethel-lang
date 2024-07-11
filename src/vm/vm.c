#include <stdio.h>
#include "../common/err.h"
#include "../comp/cg.h"
#include "../comp/comp.h"
#include "../comp/cmem.h"
#include "../comp/dis.h"
#include "../common/op.h"
#include "vm.h"

#define READ_BYTE() (*vm->pc++)

static error_t numeric_negate(vm_stack_elem_t *e) {
    switch (e->type) {
        case VM_STACK_BOOL_TYPE:
            e->boolval = e->boolval ? 0 : 1;
            break;
        case VM_STACK_BYTE_TYPE:
            e->byteval = ((0xff ^ e->byteval) + 1) & 0xff;
            break;
        case VM_STACK_INT_TYPE:
            e->intval = -e->intval;
            break;
        case VM_STACK_FLOAT_TYPE:
            e->floatval = -e->floatval;
            break;
        default:
            printf("Unsupported type for numeric negation: %d\n", e->type);
            return ERR_VM_RUNTIME_ERROR;
    }
    return ERR_NO_ERROR;
}

static error_t binop(vm_t *vm, vm_op_t op) {
    vm_stack_elem_t *a = vm_stack_pop(vm);
    vm_stack_elem_t *b = vm_stack_pop(vm);

    // TODO support more than just int.
    if (a->type != VM_STACK_INT_TYPE || b->type != VM_STACK_INT_TYPE) {
        printf("Time to implement the other types!\n");
        return ERR_VM_RUNTIME_ERROR;
    }

    vm_stack_elem_t e;
    e.type = a->type > b->type ? a->type : b->type;

    switch (op) {
        case VM_OP_ADD:
            e.intval = b->intval + a->intval;
            break;
        case VM_OP_SUB:
            e.intval = b->intval - a->intval;
            break;
        case VM_OP_MUL:
            e.intval = b->intval * a->intval;
            break;
        case VM_OP_DIV:
            e.intval = b->intval / a->intval;
            break;
        case VM_OP_REM:
            e.intval = b->intval % a->intval;
            break;
        default:
            printf("Unsupported type for numeric binary operation: %d\n", e.type);
            return ERR_VM_RUNTIME_ERROR;
    }

    // Don't free b and a: They are still slots in the stack.

    vm_stack_push(vm, &e);
    return ERR_NO_ERROR;
}

static error_t exec(vm_t *vm) {
    error_t err = ERR_NO_ERROR;
    for (;;) {
        uint8_t bytecode;
        switch (bytecode = READ_BYTE()) {
            case VM_OP_NOP:
                break;
            case VM_OP_PUSH:
                vm_stack_push_int(vm, READ_BYTE());
                break;
            case VM_OP_LOADI_1N:
                vm_stack_push_int(vm, -1);
                break;
            case VM_OP_LOADI_0:
                vm_stack_push_int(vm, 0);
                break;
            case VM_OP_LOADI_1:
                vm_stack_push_int(vm, 1);
                break;
            case VM_OP_LOADI: {
                map_elem_t v;
                uint8_t k = READ_BYTE();
                cg_get_const(vm->cg, k, &v);
                vm_stack_push_int(vm, v.elem.intval);
                break;
            }
            case VM_OP_NEGATE: {
                vm_stack_elem_t *e = vm_stack_pop(vm);
                err = numeric_negate(e);
                vm_stack_push(vm, e);
                break;
            }
            case VM_OP_ADD:
                err = binop(vm, VM_OP_ADD);
                break;
            case VM_OP_SUB:
                err = binop(vm, VM_OP_SUB);
                break;
            case VM_OP_MUL:
                err = binop(vm, VM_OP_MUL);
                break;
            case VM_OP_DIV:
                err = binop(vm, VM_OP_DIV);
                break;
            case VM_OP_REM:
                err = binop(vm, VM_OP_REM);
                break;
            case VM_OP_INC:
                vm_stack_peek(vm)->intval += 1;
                break;
            case VM_OP_DEC:
                vm_stack_peek(vm)->intval -= 1;
                break;
            case VM_OP_RET:
                return ERR_VM_INTERP_OK;
            default:
                printf("Unsupported bytecode: %d\n", bytecode);
                return ERR_VM_RUNTIME_ERROR;
        }
        if (err) {
            printf("Execution error: %d\n", err);
            return err;
        }
    }
    return err;
}

error_t vm_init(vm_t *vm) {
    cg_t *cg = comp_alloc(sizeof(cg_t));
    if (cg == NULL)
        return ERR_OUT_OF_MEMORY;
    cg_init(cg);

    // A stack of objects. Top is a pointer into buf.
    // Top always points to the next value to be filled.
    // If top == buf, stack is empty.
    vm_stack_t *stack = comp_alloc(sizeof(vm_stack_t));
    if (stack == NULL)
        return ERR_OUT_OF_MEMORY;

    vm_stack_elem_t *stack_buf = comp_alloc(sizeof(vm_stack_elem_t) * VM_DATA_STACK_SIZE);
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
    comp_free(vm->cg);
    vm_init(vm);

    return ERR_NO_ERROR;
}

error_t vm_stack_reset(vm_t *vm) {
    vm->stack->top = vm->stack->buf;
    return ERR_NO_ERROR;
}

error_t vm_stack_push(vm_t *vm, vm_stack_elem_t *e) {
    if (vm->stack->top > vm->stack->buf + sizeof(vm_stack_elem_t *) * VM_DATA_STACK_SIZE) {
        return ERR_VM_STACK_OVERFLOW;
    }
    *(vm->stack->top) = *e;
    vm->stack->top += sizeof(vm_stack_elem_t);
    return ERR_NO_ERROR;
}

error_t vm_stack_push_byte(vm_t *vm, uint8_t b) {
    vm_stack_elem_t e = {
        .type = VM_STACK_BYTE_TYPE,
        .byteval = b
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_int(vm_t *vm, int i) {
    vm_stack_elem_t e = {
        .type = VM_STACK_INT_TYPE,
        .intval = i
    };
    return vm_stack_push(vm, &e);
}

vm_stack_elem_t *vm_stack_peek(vm_t *vm) {
    if (vm->stack->top == vm->stack->buf) {
        return NULL;
    }
    return vm->stack->top - sizeof(vm_stack_elem_t);

}

vm_stack_elem_t *vm_stack_pop(vm_t *vm) {
    vm->stack->top -= sizeof(vm_stack_elem_t);
    return vm->stack->top;
}

error_t vm_load_code(vm_t *vm, uint8_t *bytecode, size_t size) {
    vm_init(vm);
    cg_bytes(vm->cg, bytecode, size);
    vm->pc = vm->cg->code;
    vm->code_size = vm->cg->len;
    return ERR_NO_ERROR;
}

error_t vm_exec(vm_t *vm) {
    printf("Executing code:\n");
    print_dis(vm->cg);
    return exec(vm);
}

error_t vm_interp(vm_t *vm, const char *input) {
    comp(input);

    return ERR_VM_INTERP_OK;
}

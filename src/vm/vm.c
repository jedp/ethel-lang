#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../common/err.h"
#include "../comp/cg.h"
#include "../comp/comp.h"
#include "../comp/cmem.h"
#include "../comp/dis.h"
#include "../common/op.h"
#include "vm.h"

#define READ_BYTE() (*vm->pc++)

void runtime_error(vm_t *vm, const char *message) {
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "Runtime error: %s\n", message);
    fflush(stderr);

    vm_stack_reset(vm);
}

bool runtime_check(vm_t *vm, bool condition, const char *message) {
    if (condition)
        return true;

    runtime_error(vm, message);

    return false;
}

static bool truthiness(vm_stack_elem_t *e) {
    switch (e->type) {
        case VM_STACK_NIL_TYPE:
            return false;
        case VM_STACK_BOOL_TYPE:
            return e->boolval == 1;
        case VM_STACK_BYTE_TYPE:
            return e->byteval != 0;
        case VM_STACK_INT_TYPE:
            return e->intval > 0;
        case VM_STACK_FLOAT_TYPE:
            return e->floatval > 0.0f;
        default:
            printf("Can't determine truthiness of stack elem type %d\n", e->type);
            return false;
    }
}

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
            return ERR_VM_RUNTIME_ERROR;
    }
    return ERR_NO_ERROR;
}

static error_t numerical_binop(vm_t *vm, vm_op_t op) {
    vm_stack_elem_t *a = vm_stack_pop(vm);
    vm_stack_elem_t *b = vm_stack_pop(vm);

    if (!(runtime_check(vm, TYPE_IS_NUMERIC(a->type) && TYPE_IS_NUMERIC(b->type),
                        "Operator requires two numeric arguments"))) {
        return ERR_VM_RUNTIME_ERROR;
    }

    // TODO support more than just int.
    if (a->type != VM_STACK_INT_TYPE || b->type != VM_STACK_INT_TYPE) {
        runtime_error(vm, "Time to implement the other types!");
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
            runtime_error(vm, "Unsupported numeric binary operation");
            return ERR_VM_RUNTIME_ERROR;
    }

    // Don't free b and a: They are still slots in the stack.

    vm_stack_push(vm, &e);
    return ERR_NO_ERROR;
}

static error_t logical_binop(vm_t *vm, vm_op_t op) {
    bool a = truthiness(vm_stack_pop(vm));
    bool b = truthiness(vm_stack_pop(vm));

    vm_stack_elem_t e;
    e.type = VM_STACK_BOOL_TYPE;

    switch(op) {
        case VM_OP_LOGICAL_AND:
            e.boolval = a && b;
            break;
        case VM_OP_LOGICAL_OR:
            e.boolval = a || b;
            break;
        default:
            runtime_error(vm, "Unsupported logical binary operation");
            return ERR_VM_RUNTIME_ERROR;
    }

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
            case VM_OP_IPUSH:
                vm_stack_push_int(vm, READ_BYTE());
                break;
            case VM_OP_IPUSH_1N:
                vm_stack_push_int(vm, -1);
                break;
            case VM_OP_IPUSH_0:
                vm_stack_push_int(vm, 0);
                break;
            case VM_OP_IPUSH_1:
                vm_stack_push_int(vm, 1);
                break;
            case VM_OP_ZPUSH_F:
                vm_stack_push_boolean(vm, false);
                break;
            case VM_OP_ZPUSH_T:
                vm_stack_push_boolean(vm, true);
                break;
            case VM_OP_ICONST: {
                map_elem_t v;
                uint8_t k = READ_BYTE();
                cg_get_const(vm->cg, k, &v);
                vm_stack_push_int(vm, v.elem.intval);
                break;
            }
            case VM_OP_TRUE:
                vm_stack_push_boolean(vm, true);
                break;
            case VM_OP_FALSE:
                vm_stack_push_boolean(vm, false);
                break;
            case VM_OP_NIL:
                vm_stack_push_nil(vm);
                break;
            case VM_OP_NEG: {
                vm_stack_elem_t *e = vm_stack_pop(vm);
                runtime_check(vm, TYPE_IS_NUMERIC(e->type), "Can only negate numbers.");
                err = numeric_negate(e);
                vm_stack_push(vm, e);
                break;
            }
            case VM_OP_ADD:
                err = numerical_binop(vm, VM_OP_ADD);
                break;
            case VM_OP_SUB:
                err = numerical_binop(vm, VM_OP_SUB);
                break;
            case VM_OP_MUL:
                err = numerical_binop(vm, VM_OP_MUL);
                break;
            case VM_OP_DIV:
                err = numerical_binop(vm, VM_OP_DIV);
                break;
            case VM_OP_REM:
                err = numerical_binop(vm, VM_OP_REM);
                break;
            case VM_OP_LOGICAL_AND:
                err = logical_binop(vm, VM_OP_LOGICAL_AND);
                break;
            case VM_OP_LOGICAL_OR:
                err = logical_binop(vm, VM_OP_LOGICAL_OR);
                break;
            case VM_OP_LOGICAL_NOT: {
                vm_stack_elem_t *e = vm_stack_pop(vm);
                bool val = truthiness(e);
                e->type = VM_STACK_BOOL_TYPE;
                e->boolval = val;
                vm_stack_push(vm, e);
                break;
            }
            case VM_OP_INC:
                vm_stack_peek(vm)->intval += 1;
                break;
            case VM_OP_DEC:
                vm_stack_peek(vm)->intval -= 1;
                break;
            case VM_OP_RET:
                return ERR_NO_ERROR;
            default:
                runtime_error(vm, "Unsupported bytecode");
                return ERR_VM_RUNTIME_ERROR;
        }
        if (err) {
            runtime_error(vm, "Execution error");
            return err;
        }
    }
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

//    vm_stack_elem_t *stack_buf = comp_alloc(sizeof(vm_stack_elem_t) * VM_DATA_STACK_SIZE);
    stack->size = 0;
    stack->top = stack->buf;

    vm->cg = cg;
    vm->pc = cg->bytecode;
    vm->bytecode_size = vm->cg->len;
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

error_t vm_stack_push_boolean(vm_t *vm, bool z) {
    vm_stack_elem_t e = {
        .type = VM_STACK_BOOL_TYPE,
        .boolval = z ? 1 : 0
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_nil(vm_t *vm) {
    vm_stack_elem_t e = {
        .type = VM_STACK_NIL_TYPE
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
    uint32_t code_start = cg_header(vm->cg, bytecode, size);
    if (code_start == 0) {
        return ERR_VM_LOAD_ERROR;
    }
    vm->cg->code_start = code_start;
    vm->pc = vm->cg->bytecode + code_start;
    vm->bytecode_size = vm->cg->len;
    return ERR_NO_ERROR;
}

error_t vm_exec(vm_t *vm) {
    printf("Executing code:\n");
    print_dis(vm->cg);
    return exec(vm);
}

error_t vm_interp(vm_t *vm, const char *input) {
    cg_t cg;
    cg_init(&cg);
    codegen(input, &cg);

    return ERR_NO_ERROR;
}

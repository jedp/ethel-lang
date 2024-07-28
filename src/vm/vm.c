#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "../common/err.h"
#include "../comp/cg.h"
#include "../comp/comp.h"
#include "../mem/mem.h"
#include "../comp/dis.h"
#include "../common/op.h"
#include "vm.h"
#include "val.h"

#define READ_BYTE() (*vm->pc++)

void runtime_error(vm_t *vm, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "Runtime error: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);

    fflush(stderr);
    va_end(args);

    vm_stack_reset(vm);
}

bool runtime_check(vm_t *vm, bool condition, const char *message) {
    if (condition)
        return true;

    runtime_error(vm, message);

    return false;
}

static bool truthiness(vm_t *vm, vm_stack_elem_t *e) {
    switch (e->type) {
        case VM_STACK_NIL_TYPE:
            return false;
        case VM_STACK_BOOL_TYPE:
            return e->as.boolval == 1;
        case VM_STACK_BYTE_TYPE:
            return e->as.byteval != 0;
        case VM_STACK_INT_TYPE:
            return e->as.intval > 0;
        case VM_STACK_FLOAT_TYPE:
            return e->as.floatval > 0.0f;
        default:
            runtime_error(vm, "Can't determine truthiness of stack elem type %d", e->type);
            return false;
    }
}

static error_t numeric_negate(vm_stack_elem_t *e) {
    switch (e->type) {
        case VM_STACK_BOOL_TYPE:
            e->as.boolval = e->as.boolval ? 0 : 1;
            break;
        case VM_STACK_BYTE_TYPE:
            e->as.byteval = ((0xff ^ e->as.byteval) + 1) & 0xff;
            break;
        case VM_STACK_INT_TYPE:
            e->as.intval = -e->as.intval;
            break;
        case VM_STACK_FLOAT_TYPE:
            e->as.floatval = -e->as.floatval;
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
            e.as.intval = b->as.intval + a->as.intval;
            break;
        case VM_OP_SUB:
            e.as.intval = b->as.intval - a->as.intval;
            break;
        case VM_OP_MUL:
            e.as.intval = b->as.intval * a->as.intval;
            break;
        case VM_OP_DIV:
            e.as.intval = b->as.intval / a->as.intval;
            break;
        case VM_OP_REM:
            e.as.intval = b->as.intval % a->as.intval;
            break;
        case VM_OP_LT:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval < a->as.intval ? 1 : 0;
            break;
        case VM_OP_LE:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval <= a->as.intval ? 1 : 0;
            break;
        case VM_OP_GT:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval > a->as.intval ? 1 : 0;
            break;
        case VM_OP_GE:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval >= a->as.intval ? 1 : 0;
            break;
        case VM_OP_EQ:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval == a->as.intval ? 1 : 0;
            break;
        case VM_OP_NE:
            e.type = VM_STACK_BOOL_TYPE;
            e.as.boolval = b->as.intval != a->as.intval ? 1 : 0;
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
    bool a = truthiness(vm, vm_stack_pop(vm));
    bool b = truthiness(vm, vm_stack_pop(vm));

    vm_stack_elem_t e;
    e.type = VM_STACK_BOOL_TYPE;

    switch (op) {
        case VM_OP_LOGICAL_AND:
            e.as.boolval = a && b;
            break;
        case VM_OP_LOGICAL_OR:
            e.as.boolval = a || b;
            break;
        default:
            runtime_error(vm, "Unsupported logical binary operation");
            return ERR_VM_RUNTIME_ERROR;
    }

    vm_stack_push(vm, &e);
    return ERR_NO_ERROR;
}

static error_t array_subscript(vm_t *vm) {
    // Subscript
    vm_stack_elem_t *a = vm_stack_pop(vm);
    // Array Object
    vm_stack_elem_t *b = vm_stack_pop(vm);

    // Must be subscriptable object.
    if (b->type != VM_STACK_OBJ_TYPE || !IS_OBJ_SUBSCRIPTABLE(b->as.objval)) {
        runtime_error(vm, "Cannot take array subscript\n");
        return ERR_VM_COMPILE_ERROR;
    }

    // Must have integer index value.
    if (!VALID_SUBSCRIPT_INDEX(a)) {
        runtime_error(vm, "Not a valid subscript index value\n");
        return ERR_VM_COMPILE_ERROR;
    }

    obj_t *b_obj = b->as.objval;
    switch (b_obj->type) {
        case OBJ_TYPE_STRING: {
            obj_str_t *str_obj = (obj_str_t *) b_obj;
            int offset = a->as.intval;
            char ch;
            if (offset >= 0) {
                // Index from start.
                if (offset >= str_obj->length) {
                    runtime_error(vm, "Subscript %d out of range for String of length %d", offset, str_obj->length);
                    return ERR_VM_COMPILE_ERROR;
                }
                ch = str_obj->chars[offset];
                vm_stack_push_byte(vm, ch);
            } else {
                // Index from end.
                if (offset < -str_obj->length) {
                    runtime_error(vm, "Subscript %d out of range for String of length %d", offset, str_obj->length);
                    return ERR_VM_COMPILE_ERROR;
                }
                ch = str_obj->chars[str_obj->length + offset];
                vm_stack_push_byte(vm, ch);
            }
            break;
        }
        case OBJ_TYPE_BYTEARRAY:
        default:
            runtime_error(vm, "Unsubscriptable object: %s\n", obj_type_names[b_obj->type]);
            return ERR_VM_COMPILE_ERROR;

    }

    return ERR_NO_ERROR;
}

static error_t jump(vm_t *vm, vm_op_t op) {
    uint16_t jump_addr = 0;
    // Little-endian
    jump_addr |= READ_BYTE();
    jump_addr |= (READ_BYTE() << 8);

    switch (op) {
        case VM_OP_JZ:
            if (!truthiness(vm, vm_stack_pop(vm))) {
                vm->pc = &(vm->cg->bytecode[jump_addr]);
            }
            break;
        case VM_OP_JEQ:
            if (truthiness(vm, vm_stack_pop(vm))) {
                vm->pc = &(vm->cg->bytecode[jump_addr]);
            }
            break;
        case VM_OP_JMP:
            vm->pc = &(vm->cg->bytecode[jump_addr]);
            break;
        default:
            runtime_error(vm, "Unsupported jump instruction");
            return ERR_VM_RUNTIME_ERROR;
    }

    return ERR_NO_ERROR;
}

static error_t exec(vm_t *vm) {
    error_t err = ERR_NO_ERROR;
    for (;;) {
        uint8_t bytecode = READ_BYTE();
        /*
        if (bytecode < VM_OP_MAX) {
            printf("pc 0x%lx, code %s\n", (vm->pc - 1 - vm->cg->bytecode), op_names[bytecode]);
        } else {
            printf("pc 0x%lx, code %d\n", (vm->pc - 1 - vm->cg->bytecode), bytecode);
        }
         */
        switch (bytecode) {
            case VM_OP_NOP:
                break;
            case VM_OP_IPUSH:
                vm_stack_push_byte_as_int32(vm, READ_BYTE());
                break;
            case VM_OP_IPUSH_1N:
                vm_stack_push_int32(vm, -1);
                break;
            case VM_OP_IPUSH_0:
                vm_stack_push_int32(vm, 0);
                break;
            case VM_OP_IPUSH_1:
                vm_stack_push_int32(vm, 1);
                break;
            case VM_OP_ZPUSH_F:
                vm_stack_push_boolean(vm, false);
                break;
            case VM_OP_ZPUSH_T:
                vm_stack_push_boolean(vm, true);
                break;
            case VM_OP_ICONST: {
                val_t v;
                uint8_t k = READ_BYTE();
                cg_get_const(vm->cg, k, &v);
                vm_stack_push_int32(vm, v.as.intval);
                break;
            }
            case VM_OP_SCONST:
            case VM_OP_ACONST: {
                val_t v;
                uint8_t k = READ_BYTE();
                cg_get_const(vm->cg, k, &v);
                vm_stack_push_obj(vm, v.as.objval);
                break;
            }
            case VM_OP_AALLOC: {
                vm_stack_elem_t *size_obj = vm_stack_pop(vm);
                runtime_check(vm, TYPE_IS_NUMERIC(size_obj->type), "Numeric object required for array size.");
                obj_arr_t *obj_arr = obj_arr_new(NULL, size_obj->as.intval);
                vm_stack_push_obj(vm, (obj_t *) obj_arr);
                break;
            }
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
            case VM_OP_ALOAD:
                err = array_subscript(vm);
                break;
            case VM_OP_LOGICAL_AND:
                err = logical_binop(vm, VM_OP_LOGICAL_AND);
                break;
            case VM_OP_LOGICAL_OR:
                err = logical_binop(vm, VM_OP_LOGICAL_OR);
                break;
            case VM_OP_LT:
                err = numerical_binop(vm, VM_OP_LT);
                break;
            case VM_OP_LE:
                err = numerical_binop(vm, VM_OP_LE);
                break;
            case VM_OP_GT:
                err = numerical_binop(vm, VM_OP_GT);
                break;
            case VM_OP_GE:
                err = numerical_binop(vm, VM_OP_GE);
                break;
            case VM_OP_EQ:
                err = numerical_binop(vm, VM_OP_EQ);
                break;
            case VM_OP_NE:
                err = numerical_binop(vm, VM_OP_NE);
                break;
            case VM_OP_LOGICAL_NOT: {
                vm_stack_elem_t *e = vm_stack_pop(vm);
                bool val = truthiness(vm, e);
                e->type = VM_STACK_BOOL_TYPE;
                e->as.boolval = val;
                vm_stack_push(vm, e);
                break;
            }
            case VM_OP_INC:
                vm_stack_peek(vm)->as.intval += 1;
                break;
            case VM_OP_DEC:
                vm_stack_peek(vm)->as.intval -= 1;
                break;
            case VM_OP_PRINT: {
                char *string = mem_alloc(sizeof(char) * 255);
                string[255] = '\0';
                vm_print_val(vm_stack_pop(vm), string, 254);
                printf("%s\n", string);
                break;
            }
            case VM_OP_RET:
                return ERR_NO_ERROR;
            case VM_OP_JZ:
            case VM_OP_JEQ:
            case VM_OP_JMP:
                err = jump(vm, bytecode);
                break;
            default:
                runtime_error(vm, "Unsupported bytecode: %d", bytecode);
                return ERR_VM_RUNTIME_ERROR;
        }
        if (err) {
            return err;
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
    mem_free(vm->cg);
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
        .as.byteval = b
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_byte_as_int32(vm_t *vm, uint8_t b) {
    int signed_val = b;
    if (b & 0x80) {
        signed_val = -1;
        signed_val <<= 8;
        signed_val |= b;
    }
    vm_stack_elem_t e = {
        .type = VM_STACK_INT_TYPE,
        .as.intval = signed_val,
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_int32(vm_t *vm, int i) {
    vm_stack_elem_t e = {
        .type = VM_STACK_INT_TYPE,
        .as.intval = i,
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_boolean(vm_t *vm, bool z) {
    vm_stack_elem_t e = {
        .type = VM_STACK_BOOL_TYPE,
        .as.boolval = z ? 1 : 0
    };
    return vm_stack_push(vm, &e);
}

error_t vm_stack_push_obj(vm_t *vm, obj_t *obj) {
    vm_stack_elem_t e = {
        .type=VM_STACK_OBJ_TYPE,
        .as.objval = obj
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

uint8_t vm_stack_size(vm_t *vm) {
    return (vm->stack->top - vm->stack->buf) / sizeof(vm_stack_elem_t);
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

void vm_print_val(const vm_stack_elem_t *elem, char *string, uint8_t max_length) {
    switch (elem->type) {
        case VM_STACK_NIL_TYPE:
            snprintf(string, max_length, "<nil>");
            break;
        case VM_STACK_BOOL_TYPE:
            snprintf(string, max_length, (elem->as.boolval) ? "true" : "false");
            break;
        case VM_STACK_BYTE_TYPE:
            snprintf(string, max_length, "0x%02x", elem->as.byteval);
            break;
        case VM_STACK_INT_TYPE: {
            snprintf(string, max_length, "%d", elem->as.intval);
            break;
        }
        case VM_STACK_FLOAT_TYPE: {
            sprintf(string, "%f", elem->as.floatval);
            break;
        }
        case VM_STACK_OBJ_TYPE: {
            obj_t *obj = elem->as.objval;
            switch (obj->type) {
                case OBJ_TYPE_STRING: {
                    obj_str_t *obj_str = (obj_str_t *) obj;
                    snprintf(string, max_length, "%s%c", obj_str->chars, '\0');
                    break;
                }
                case OBJ_TYPE_BYTEARRAY: {
                    obj_arr_t *obj_arr = (obj_arr_t *) obj;
                    snprintf(string, max_length, "array(%d)%c", obj_arr->length, '\0');
                    break;
                }
                default: {
                    snprintf(string, max_length, "Unprintable object");
                    break;
                }
            }
        }
        case VM_STACK_ADDR_TYPE:
        default: {
            string = "Unprintable type";
            break;
        }
    }
}

error_t vm_exec(vm_t *vm) {
    print_dis(vm->cg);
    printf("Executing ...\n");
    return exec(vm);
}

error_t vm_interp(vm_t *vm, const char *input) {
    cg_t cg;
    cg_init(&cg);
    codegen(input, &cg);

    return ERR_NO_ERROR;
}

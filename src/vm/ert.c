#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../mem/mem.h"
#include "../dis/dis.h"
#include "../common/op.h"
#include "ert.h"
#include "vm.h"
#include "val.h"

#define READ_BYTE() (*vm->pc++)

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

static vm_err_t numeric_negate(vm_stack_elem_t *e) {
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
            return VM_ERR_RUNTIME_ERROR;
    }
    return VM_ERR_NO_ERROR;
}

static vm_err_t numerical_binop(vm_t *vm, vm_op_t op) {
    vm_stack_elem_t *a = vm_stack_pop(vm);
    vm_stack_elem_t *b = vm_stack_pop(vm);

    if (!(runtime_check(vm, TYPE_IS_NUMERIC(a->type) && TYPE_IS_NUMERIC(b->type),
                        "Operator requires two numeric arguments"))) {
        return VM_ERR_RUNTIME_ERROR;
    }

    // TODO support more than just int.
    if (a->type != VM_STACK_INT_TYPE || b->type != VM_STACK_INT_TYPE) {
        runtime_error(vm, "Time to implement the other types!");
        return VM_ERR_RUNTIME_ERROR;
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
        case VM_OP_BIN_SHL:
            e.as.intval = b->as.intval << a->as.intval;
            break;
        case VM_OP_BIN_SHR:
            e.as.intval = b->as.intval >> a->as.intval;
            break;
        case VM_OP_BIN_AND:
            e.as.intval = b->as.intval & a->as.intval;
            break;
        case VM_OP_BIN_OR:
            e.as.intval = b->as.intval | a->as.intval;
            break;
        case VM_OP_BIN_XOR:
            e.as.intval = b->as.intval ^ a->as.intval;
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
            return VM_ERR_RUNTIME_ERROR;
    }

    // Don't free b and a: They are still slots in the stack.

    vm_stack_push(vm, &e);
    return VM_ERR_NO_ERROR;
}

static vm_err_t logical_binop(vm_t *vm, vm_op_t op) {
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
            return VM_ERR_RUNTIME_ERROR;
    }

    vm_stack_push(vm, &e);
    return VM_ERR_NO_ERROR;
}

static vm_err_t array_subscript(vm_t *vm) {
    // Subscript
    vm_stack_elem_t *a = vm_stack_pop(vm);
    // Array Object
    vm_stack_elem_t *b = vm_stack_pop(vm);

    // Must be subscriptable object.
    if (b->type != VM_STACK_OBJ_TYPE || !IS_OBJ_SUBSCRIPTABLE(b->as.objval)) {
        runtime_error(vm, "Cannot take array subscript\n");
        return VM_ERR_COMPILE_ERROR;
    }

    // Must have integer index value.
    if (!VALID_SUBSCRIPT_INDEX(a)) {
        runtime_error(vm, "Not a valid subscript index value\n");
        return VM_ERR_COMPILE_ERROR;
    }

    obj_t *b_obj = b->as.objval;
    int offset = a->as.intval;
    switch (b_obj->type) {
        case OBJ_TYPE_STRING: {
            obj_str_t *obj_str = (obj_str_t *) b_obj;
            char ch;
            if (offset >= 0) {
                // Index from start.
                if (offset >= obj_str->length) {
                    goto obj_index_err;
                }
                ch = obj_str->chars[offset];
                vm_stack_push_byte(vm, ch);
            } else {
                // Index from end.
                if (offset < -obj_str->length) {
                    goto obj_index_err;
                }
                ch = obj_str->chars[obj_str->length + offset];
                vm_stack_push_byte(vm, ch);
            }
            break;
        }
        case OBJ_TYPE_BYTEARRAY: {
            obj_arr_t *obj_arr = (obj_arr_t *) b_obj;
            uint8_t byte;
            if (offset >= 0) {
                // Index from start.
                if (offset >= obj_arr->length) {
                    goto obj_index_err;
                }
                byte = obj_arr->buf[offset];
                vm_stack_push_byte(vm, byte);
            } else {
                // Index from end.
                if (offset < -obj_arr->length) {
                    goto obj_index_err;
                }
                byte = obj_arr->buf[obj_arr->length + offset];
                vm_stack_push_byte(vm, byte);
            }
            break;
        }
        default:
            runtime_error(vm, "Unsubscriptable object: %s\n", obj_type_names[b_obj->type]);
            return VM_ERR_COMPILE_ERROR;
    }

    return VM_ERR_NO_ERROR;

    obj_index_err:
    // We can get the length attribute of any indexable object (str or arr).
    // So as a hack, just cast it to an array to get length.
    runtime_error(vm, "Subscript %d out of range for %s of length %d",
                  offset, obj_type_names[b_obj->type], ((obj_arr_t *) b_obj)->length);
    return VM_ERR_COMPILE_ERROR;
}

static vm_err_t jump(vm_t *vm, vm_op_t op) {
    uint16_t jump_addr = 0;
    // Little-endian
    jump_addr |= READ_BYTE();
    jump_addr |= (READ_BYTE() << 8);

    switch (op) {
        case VM_OP_JZ:
            if (!truthiness(vm, vm_stack_pop(vm))) {
                vm->pc = &(vm->bytecode[jump_addr]);
            }
            break;
        case VM_OP_JEQ:
            if (truthiness(vm, vm_stack_pop(vm))) {
                vm->pc = &(vm->bytecode[jump_addr]);
            }
            break;
        case VM_OP_JMP:
            vm->pc = &(vm->bytecode[jump_addr]);
            break;
        default:
            runtime_error(vm, "Unsupported jump instruction");
            return VM_ERR_RUNTIME_ERROR;
    }

    return VM_ERR_NO_ERROR;
}

static vm_err_t exec(vm_t *vm) {
    vm_err_t err = VM_ERR_NO_ERROR;
    for (;;) {
        uint8_t bytecode = READ_BYTE();
        /*
        if (bytecode < VM_OP_MAX) {
            printf("pc 0x%lx, code %s\n", (vm->pc - 1 - vm->bytecode), op_names[bytecode]);
        } else {
            printf("pc 0x%lx, code %d\n", (vm->pc - 1 - vm->bytecode), bytecode);
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
                vm_map_get(vm->consts, k, &v);
                vm_stack_push_int32(vm, v.as.intval);
                break;
            }
            case VM_OP_SCONST:
            case VM_OP_ACONST: {
                val_t v;
                uint8_t k = READ_BYTE();
                vm_map_get(vm->consts, k, &v);
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
            case VM_OP_BIN_SHL:
                err = numerical_binop(vm, VM_OP_BIN_SHL);
                break;
            case VM_OP_BIN_SHR:
                err = numerical_binop(vm, VM_OP_BIN_SHR);
                break;
            case VM_OP_BIN_AND:
                err = numerical_binop(vm, VM_OP_BIN_AND);
                break;
            case VM_OP_BIN_OR:
                err = numerical_binop(vm, VM_OP_BIN_OR);
                break;
            case VM_OP_BIN_XOR:
                err = numerical_binop(vm, VM_OP_BIN_XOR);
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
                return VM_ERR_NO_ERROR;
            case VM_OP_JZ:
            case VM_OP_JEQ:
            case VM_OP_JMP:
                err = jump(vm, bytecode);
                break;
            default:
                runtime_error(vm, "Unsupported bytecode: %d", bytecode);
                return VM_ERR_RUNTIME_ERROR;
        }
        if (err) {
            return err;
        }
    }
}

vm_err_t vm_stack_push(vm_t *vm, vm_stack_elem_t *e) {
    if (vm->stack->top > vm->stack->buf + sizeof(vm_stack_elem_t *) * VM_DATA_STACK_SIZE) {
        return VM_ERR_STACK_OVERFLOW;
    }
    *(vm->stack->top) = *e;
    vm->stack->top += sizeof(vm_stack_elem_t);
    return VM_ERR_NO_ERROR;
}

vm_err_t vm_stack_push_byte(vm_t *vm, uint8_t b) {
    vm_stack_elem_t e = {
        .type = VM_STACK_BYTE_TYPE,
        .as.byteval = b
    };
    return vm_stack_push(vm, &e);
}

vm_err_t vm_stack_push_byte_as_int32(vm_t *vm, uint8_t b) {
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

vm_err_t vm_stack_push_int32(vm_t *vm, int i) {
    vm_stack_elem_t e = {
        .type = VM_STACK_INT_TYPE,
        .as.intval = i,
    };
    return vm_stack_push(vm, &e);
}

vm_err_t vm_stack_push_boolean(vm_t *vm, bool z) {
    vm_stack_elem_t e = {
        .type = VM_STACK_BOOL_TYPE,
        .as.boolval = z ? 1 : 0
    };
    return vm_stack_push(vm, &e);
}

vm_err_t vm_stack_push_obj(vm_t *vm, obj_t *obj) {
    vm_stack_elem_t e = {
        .type=VM_STACK_OBJ_TYPE,
        .as.objval = obj
    };
    return vm_stack_push(vm, &e);
}

vm_err_t vm_stack_push_nil(vm_t *vm) {
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
            snprintf(string, max_length, "Unprintable type");
            break;
        }
    }
}

vm_err_t vm_exec(vm_t *vm) {
    dis_data_t dis = {
        .bytecode = vm->bytecode,
        .length = vm->bytecode_size,
        .code_start = vm->code_start,
        .consts = vm->consts,
    };
    print_dis(&dis);

    printf("Executing ...\n");
    return exec(vm);
}

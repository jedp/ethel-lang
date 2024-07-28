#include <stdio.h>
#include <stdlib.h>

#include "cg.h"
#include "comp.h"
#include "../common/op.h"
#include "dis.h"
#include "val.h"

static uint32_t print_op(const char *name, uint32_t offset) {
    printf("%-12s\n", name);
    return offset + 1;
}

static uint32_t print_imm(const char *name, cg_t *cg, uint32_t offset) {
    return print_op(name, offset);
}

static uint32_t print_push(const char *name, cg_t *cg, uint32_t offset) {
    printf("%-12s [0x%x]\n", name, cg->bytecode[offset + 1]);
    return offset + 2;
}

static uint32_t print_loadi(const char *name, cg_t *cg, uint32_t offset) {
    val_t v;
    cg_get_const(cg, cg->bytecode[offset + 1], &v);
    printf("%-12s #%02x [%d]\n", name, cg->bytecode[offset + 1], v.as.intval);
    return offset + 2;
}

static uint32_t print_loads(const char *name, cg_t *cg, uint32_t offset) {
    val_t v;
    cg_get_const(cg, cg->bytecode[offset + 1], &v);
    printf("%-12s #%x [%s]\n", name, cg->bytecode[offset + 1], obj_str_to_c(AS_OBJ_STR(&v)));
    return offset + 2;
}

static uint32_t print_loada(const char *name, cg_t *cg, uint32_t offset) {
    uint8_t truncate_after = 4;
    val_t v;
    cg_get_const(cg, cg->bytecode[offset + 1], &v);
    obj_arr_t *obj_arr = AS_OBJ_ARR(&v);
    printf("%-12s #%x (%d){", name, cg->bytecode[offset + 1], obj_arr->length);
    uint8_t last = (truncate_after < obj_arr->length) ? truncate_after : obj_arr->length;
    bool is_truncated = last != obj_arr->length;
    for (uint8_t i = 0; i < last; i++) {
        printf("%d", obj_arr->buf[i]);
        if (i != last - 1) {
            printf(",");
        }
    }
    if (is_truncated) {
        printf(", ..., %d}\n", obj_arr->buf[obj_arr->length - 1]);
    } else {
        printf("}\n");
    }
    return offset + 2;
}

static uint32_t print_jump(const char *name, cg_t *cg, uint32_t offset) {
    printf("%-12s @0x%02x%02x\n",
           name,
           cg->bytecode[offset + 2],
           cg->bytecode[offset + 1]
    );
    // 16 bit addresses, so skip two bytes.
    return offset + 3;
}

uint32_t print_dis_byte(cg_t *cg, uint32_t offset) {
    printf("%08x %02x ", offset, cg->bytecode[offset]);

    uint8_t op = cg->bytecode[offset];
    switch (op) {
        case VM_OP_IPUSH_1N:
        case VM_OP_IPUSH_0:
        case VM_OP_IPUSH_1:
        case VM_OP_ZPUSH_F:
        case VM_OP_ZPUSH_T:
            return print_imm(op_names[op], cg, offset);
        case VM_OP_BPUSH:
        case VM_OP_IPUSH:
            return print_push(op_names[op], cg, offset);
        case VM_OP_ICONST:
            return print_loadi(op_names[op], cg, offset);
        case VM_OP_SCONST:
            return print_loads(op_names[op], cg, offset);
        case VM_OP_ACONST:
            return print_loada(op_names[op], cg, offset);
        case VM_OP_AALLOC:
        case VM_OP_ALOAD:
        case VM_OP_NOP:
        case VM_OP_RET:
        case VM_OP_NEG:
        case VM_OP_NIL:
        case VM_OP_LOGICAL_AND:
        case VM_OP_LOGICAL_OR:
        case VM_OP_LOGICAL_NOT:
        case VM_OP_GT:
        case VM_OP_GE:
        case VM_OP_LT:
        case VM_OP_LE:
        case VM_OP_EQ:
        case VM_OP_NE:
        case VM_OP_ADD:
        case VM_OP_SUB:
        case VM_OP_MUL:
        case VM_OP_DIV:
        case VM_OP_REM:
        case VM_OP_BIN_OR:
        case VM_OP_BIN_XOR:
        case VM_OP_BIN_AND:
        case VM_OP_BIN_SHL:
        case VM_OP_BIN_SHR:
        case VM_OP_INC:
        case VM_OP_DEC:
        case VM_OP_ASSIGN:
            return print_op(op_names[op], offset);
        case VM_OP_JEQ:
        case VM_OP_JZ:
        case VM_OP_JMP:
            return print_jump(op_names[op], cg, offset);
        default:
            printf("unknown op %d\n", op);
            return print_op("**UNKNOWN**", offset);
    }
}

static void print_constants(cg_t *cg) {
    uint8_t offset = 6;
    uint8_t num_consts = cg->bytecode[offset++];
    printf("= Constants: %d\n", num_consts);

    // 1-indexed constants.
    for (uint8_t i = 1; i <= num_consts; i++) {
        uint8_t type = cg->bytecode[offset++];
        printf("#%02d ", i);
        switch (type) {
            case CONST_INT: {
                uint8_t int_len = cg->bytecode[offset++];
                printf("INT%d  ", (int_len * 8));
                for (int j = 0; j < int_len; j++) {
                    printf("0x%02x ", cg->bytecode[offset++]);
                }
                printf("\n");
                break;
            }
            case CONST_STRING : {
                uint8_t str_len = cg->bytecode[offset++];
                printf("STR  \"");
                for (uint32_t j = 0; j < str_len; j++) {
                    uint8_t c = cg->bytecode[offset++];
                    printf("%c", (c >= 32 && c <= 126) ? (char) c : '.');
                }
                printf("\"\n");
                break;
            }
            case CONST_BYTEARRAY: {
                // Four-byte array length, little-endian.
                uint32_t arr_len = 0;
                arr_len |= cg->bytecode[offset++];
                arr_len |= cg->bytecode[offset++] << 8;
                arr_len |= cg->bytecode[offset++] << 16;
                arr_len |= cg->bytecode[offset++] << 24;
                printf("ARR(%d) ", arr_len);
                for (uint32_t j = 0; j < arr_len; j++) {
                    printf(" %d", cg->bytecode[offset++]);
                }
                printf("\n");
                break;
            }

            default:
                printf("ERROR: Can't disassemble const type %d\n", type);
                exit(1);
        }
    }
}

static void print_header(cg_t *cg) {
    if (cg->code_start > 0) {
        printf("= Header %c%c%c%c v%d.%d\n",
               cg->bytecode[0], cg->bytecode[1], cg->bytecode[2], cg->bytecode[3],
               cg->bytecode[4], cg->bytecode[5]
        );

        print_constants(cg);
    }
}

void print_dis(cg_t *cg) {
    printf("\n== Disassembly ==\n");

    print_header(cg);

    printf("= Code (start 0x%x, end 0x%x)\n", cg->code_start, cg->len - 1);
    printf("%8s %s\n", "Offset", "Instruction");
    for (uint32_t offset = cg->code_start; offset < cg->len;) {
        offset = print_dis_byte(cg, offset);
    }
}

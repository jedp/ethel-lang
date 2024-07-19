#include <stdio.h>

#include "cg.h"
#include "../common/op.h"
#include "dis.h"

static uint32_t print_op(const char *name, uint32_t offset) {
    printf("%8s\n", name);
    return offset + 1;
}

static uint32_t print_push(const char *name, cg_t *cg, uint32_t offset) {
    printf("%8s [0x%02x]\n", name, cg->code[offset + 1]);
    return offset + 2;
}

static uint32_t print_loadi(const char *name, cg_t *cg, uint32_t offset) {
    map_elem_t v;
    cg_get_const(cg, cg->code[offset + 1], &v);
    printf("%8s #%d [%d]\n", name, cg->code[offset + 1], v.elem.intval);
    return offset + 2;
}

static uint32_t print_loads(const char *name, cg_t *cg, uint32_t offset) {
    map_elem_t v;
    cg_get_const(cg, cg->code[offset + 1], &v);
    printf("%8s #%d [%s]\n", name, cg->code[offset + 1], v.elem.stringval_ptr);
    return offset + 2;
}

uint32_t print_dis_byte(cg_t *cg, uint32_t offset) {
    printf("%08d %02x ", offset, cg->code[offset]);

    uint8_t op = cg->code[offset];
    switch (op) {
        case VM_OP_NOP:
        case VM_OP_RET:
        case VM_OP_NEG:
        case VM_OP_LOADI_1N:
        case VM_OP_LOADI_0:
        case VM_OP_LOADI_1:
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
        case VM_OP_PUSHI:
            return print_push(op_names[op], cg, offset);
        case VM_OP_LOADI:
            return print_loadi(op_names[op], cg, offset);
        case VM_OP_LOADS:
            return print_loads(op_names[op], cg, offset);
        default:
            return print_op("**UNKNOWN**", offset);
    }
}

void print_dis(cg_t *cg) {
    printf("\n== Disassembly ==\n");
    printf("%8s %s\n", "Offset", "Instruction");
    for (uint32_t offset = 0; offset < cg->len;) {
        offset = print_dis_byte(cg, offset);
    }
}

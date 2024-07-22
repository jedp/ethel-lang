#include <stdio.h>

#include "cg.h"
#include "../common/op.h"
#include "dis.h"

static uint32_t print_op(const char *name, uint32_t offset) {
    printf("%8s\n", name);
    return offset + 1;
}

static uint32_t print_imm(const char *name, cg_t *cg, uint32_t offset) {
    return print_op(name, offset);
}

static uint32_t print_push(const char *name, cg_t *cg, uint32_t offset) {
    printf("%8s [0x%x]\n", name, cg->bytecode[offset + 1]);
    return offset + 2;
}

static uint32_t print_loadi(const char *name, cg_t *cg, uint32_t offset) {
    map_elem_t v;
    cg_get_const(cg, cg->bytecode[offset + 1], &v);
    printf("%8s #%02x [%d]\n", name, cg->bytecode[offset + 1], v.elem.intval);
    return offset + 2;
}

static uint32_t print_loads(const char *name, cg_t *cg, uint32_t offset) {
    map_elem_t v;
    cg_get_const(cg, cg->bytecode[offset + 1], &v);
    printf("%8s #%x [%s]\n", name, cg->bytecode[offset + 1], v.elem.stringval_ptr);
    return offset + 2;
}

static uint32_t print_jump(const char *name, cg_t *cg, uint32_t offset) {
    printf("%8s 0x%02x%02x\n",
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
        case VM_OP_IPUSH:
            return print_push(op_names[op], cg, offset);
        case VM_OP_ICONST:
            return print_loadi(op_names[op], cg, offset);
        case VM_OP_SCONST:
            return print_loads(op_names[op], cg, offset);
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

void print_dis(cg_t *cg) {
    printf("\n== Disassembly ==\n");

    printf("= Header %c%c%c%c v%d.%d\n",
           cg->bytecode[0], cg->bytecode[1], cg->bytecode[2], cg->bytecode[3],
           cg->bytecode[4], cg->bytecode[5]
           );

    uint8_t num_consts = cg->bytecode[6];
    printf("= Constants: %d\n", num_consts);

    printf("= Code (start 0x%x, end 0x%x)\n", cg->code_start, cg->len - 1);
    printf("%8s %s\n", "Offset", "Instruction");
    for (uint32_t offset = cg->code_start; offset < cg->len;) {
        offset = print_dis_byte(cg, offset);
    }
}

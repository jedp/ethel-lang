#include <stdio.h>

#include "dict.h"
#include "dis.h"
#include "op.h"

static uint32_t print_op(const char *name, uint32_t offset) {
    printf("%8s\n", name);
    return offset + 1;
}

static uint32_t print_loadi(const char *name, cg_t *cg, uint32_t offset) {
    obj_t *const_obj = dict_get(cg->consts, int_obj(cg->code[offset + 1]));
    printf("%8s %4d [%d]\n", name, cg->code[offset + 1], const_obj->intval);
    return offset + 2;
}

uint32_t print_dis_byte(cg_t *cg, uint32_t offset) {
    printf("%08d ", offset);

    uint8_t op = cg->code[offset];
    switch (op) {
        case VM_OP_NOP:
        case VM_OP_RET:
            return print_op(op_names[op], offset);
        case VM_OP_LOADI:
            return print_loadi(op_names[op], cg, offset);
        default:
            return print_op("**UNKNOWN**", offset);
    }
}

void print_dis(cg_t *cg) {
    printf("%8s %s\n", "Offset", "Instruction");
    for (uint32_t offset = 0; offset < cg->len;) {
        offset = print_dis_byte(cg, offset);
    }
}

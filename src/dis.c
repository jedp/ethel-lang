#include <stdlib.h>
#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/dis.h"

static uint32_t dis_op(cg_t *cg, uint32_t offset)
{
    printf("%04d ", offset);

    uint8_t op = bytearray_get(cg->code, offset);
    switch(op) {
        case OP_RET:
            printf("%s\n", op_names[op]);
            return offset + 1;
        case OP_CONST:
            printf("%s\t%d\n", op_names[op], bytearray_get(cg->code, offset + 1));
            return offset + 2;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            printf("%s\n", op_names[op]);
            return offset + 1;
        default:
            printf("Unknown opcode: %d\n", op);
            exit(1);
    }
}

void dis(vm_t *vm, const char *name)
{
    printf("Disassembly of %s:\n", name);

    for (uint32_t i = 0; i < vm->cg->code_len;) {
        i = dis_op(vm->cg, i);
    }
}

int main(int argc, const char **argv)
{
    (void) argc;
    (void) argv;

    mem_init('Q');

    vm_t *vm;
    vm = mem_alloc(sizeof(vm_t));

    vm_reset(vm);

    // Hello, world.
    obj_t *one = int_obj(1);
    obj_t *two = int_obj(2);
    add_const(vm, two);
    add_const(vm, one);
    add_const(vm, one);
    cg_byte(vm, OP_ADD);
    cg_byte(vm, OP_MUL);
    cg_byte(vm, OP_RET);

    dis(vm, "Hello, world!");
    return 0;
}
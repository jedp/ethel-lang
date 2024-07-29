#include "unity/unity.h"
#include "test_dis.h"

#include "../src/dis/dis.h"
#include "../src/comp/comp.h"
#include "../src/common/op.h"

void test_print_dis(void) {
    cg_t cg;
    cg_init(&cg);

    // Header
    uint8_t bytecode[] = {
        // Header
        'E', 'T', 'H', 'L', 0, 1,

        // 2 Consts
        2,
        CONST_INT, 3, 1, 1, 1,
        CONST_STRING, 3, 'f', 'o', 'o',

        // Program
        VM_OP_IPUSH, 0xa5,
        VM_OP_ICONST, 0x01,
        VM_OP_ADD,
        VM_OP_RET,
    };

    cg_bytes(&cg, bytecode, sizeof(bytecode));
    cg.code_start = 17;

    dis_data_t dis = {
        .bytecode = cg.bytecode,
        .consts = cg.consts,
        .code_start = cg.code_start,
        .length = cg.len
    };

    print_dis(&dis);

    cg_free(&cg);
}

void test_dis(void) {
    RUN_TEST(test_print_dis);
}

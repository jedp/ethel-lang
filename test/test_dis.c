#include "unity/unity.h"
#include "test_dis.h"

#include "../inc/op.h"
#include "../inc/cg.h"
#include "../inc/dis.h"

void test_print_dis(void) {
    cg_t cg;
    cg_init(&cg);

    cg_add_const(&cg, int_obj(1), int_obj(42));

    cg_byte(&cg, VM_OP_LOADI);
    cg_byte(&cg, 1);
    cg_byte(&cg, VM_OP_NOP);
    cg_byte(&cg, VM_OP_RET);

    print_dis(&cg);

    cg_free(&cg);
}

void test_dis() {
    RUN_TEST(test_print_dis);
}

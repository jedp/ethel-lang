#include "unity/unity.h"
#include "test_dis.h"

#include "../src/comp/cg.h"
#include "../src/vm/dis.h"
#include "../src/vm/map.h"
#include "../src/vm/op.h"

void test_print_dis(void) {
    cg_t cg;
    cg_init(&cg);

    cg_add_const(&cg, 1, 42);

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

#include "unity/unity.h"
#include "test_dis.h"

#include "../src/comp/cg.h"
#include "../src/common/op.h"
#include "../src/comp/dis.h"

void test_print_dis(void) {
    cg_t cg;
    cg_init(&cg);

    uint8_t k;
    map_elem_t v = {.type= MAP_ELEM_INT_TYPE, .elem.intval = 42};
    cg_put_const(&cg, v, &k);

    cg_byte(&cg, VM_OP_PUSH);
    cg_byte(&cg, 0xa5);
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

#include "unity/unity.h"
#include "test_ptr.h"
#include "test_lex.h"
#include "test_cg.h"
#include "test_dis.h"
#include "test_vm.h"
#include "test_map.h"
#include "test_comp.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    test_ptr();
    test_lex();
    test_dis();
    test_map();
    test_cg();
    test_comp();
    test_vm();

    UNITY_END();
}

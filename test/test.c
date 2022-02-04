#include "../inc/mem.h"
#include "unity/unity.h"
#include "test_heap.h"
#include "test_gc.h"
#include "test_ptr.h"
#include "test_trace.h"
#include "test_str.h"
#include "test_range.h"
#include "test_list.h"
#include "test_bytearray.h"
#include "test_dict.h"
#include "test_lexer.h"
#include "test_parser.h"
#include "test_eval.h"
#include "test_hash.h"
#include "test_env.h"
#include "test_rand.h"
#include "test_examples.h"

void setUp(void) {
    mem_init('x');
}

void tearDown(void) {
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    test_lexer();
    test_parser();
    test_heap();
    test_gc();
    test_ptr();
    test_trace();
    test_str();
    test_range();
    test_list();
    test_dict();
    test_bytearray();
    test_hash();
    test_env();
    test_eval();
    test_rand();
    test_examples();

    UNITY_END();
}

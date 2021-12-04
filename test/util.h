#ifndef __TEST_UTIL_H
#define __TEST_UTIL_H

#include "../inc/obj.h"
#include "../inc/eval.h"

obj_method_args_t *n_args(int n, ...);

obj_t *make_list(int n_elems, ...);

eval_result_t *eval_program(char* program);

#endif


#pragma once

#include "../inc/obj.h"
#include "../inc/eval.h"

obj_varargs_t *n_args(int n, ...);

obj_t *make_list(int n_elems, ...);

void eval_program(const char *program, eval_result_t *result);

#include <stdarg.h>
#include <stdio.h>
#include "util.h"
#include "../inc/ptr.h"
#include "../inc/str.h"
#include "../inc/mem.h"

obj_varargs_t *n_args(int n, ...) {
    va_list vargs;
    va_start(vargs, n);

    obj_varargs_t *args = (obj_varargs_t *) alloc_type(TYPE_VARIABLE_ARGS, F_NONE);
    obj_varargs_t *root = args;

    for (int i = 0; i < n; i++) {
        int val = va_arg(vargs, int);
        args->arg = int_obj(val);

        if (i < n - 1) {
            args->next = (obj_varargs_t *) alloc_type(TYPE_VARIABLE_ARGS, F_NONE);
        } else {
            args->next = NULL;
        }

        args = args->next;
    }

    return root;
}

// A clone of the equivalent function in eval.c
obj_t *make_list(int n_elems, ...) {
    if (n_elems == 0) {
        return list_obj(NULL);
    }

    obj_list_element_t *elem = (obj_list_element_t *) alloc_type(TYPE_LIST_ELEM_DATA, F_NONE);
    obj_list_element_t *root_elem = elem;

    elem->node = NULL;
    elem->next = NULL;

    va_list vargs;
    va_start(vargs, n_elems);

    for (int i = 0; i < n_elems; i++) {
        int val = va_arg(vargs, int);
        elem->node = int_obj(val);
        if (i < n_elems - 1) {
            elem->next = (obj_list_element_t *) alloc_type(TYPE_LIST_ELEM_DATA, F_NONE);
        } else {
            elem->next = NULL;
        }

        elem = elem->next;
    }

    va_end(vargs);

    return list_obj(root_elem);
}

void eval_program(const char *program, eval_result_t *result) {
    interp_t interp;
    interp_init(&interp);

    // Move input to ethel's memory.
    size_t len = c_str_len(program) + 1;
    char *buf = mem_alloc(len);
    mem_cp(buf, (void *) program, len);

    put_env_gc_root(&interp, (gc_header_t *) result);
    enter_scope(&interp);

    eval(&interp, program, result);

    if (result->err != ERR_NO_ERROR) {
        printf("Error executing program: %s\n", err_names[result->err]);
    }
}

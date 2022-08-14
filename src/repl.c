#include <stdlib.h>
#include <stdio.h>
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/int.h"
#include "../inc/str.h"
#include "../inc/dict.h"
#include "../inc/type.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define MAX_INPUT 1024

/*
 * TODO this repl is a big hack that uses both the stdlib memory manager
 * and the ethel memory manager. The TODO is to stop being lazy and do it
 * all with ethel.
 */
char input[MAX_INPUT] = "";

static void print_value(obj_t *obj) {
    static_method to_string = get_static_method(TYPEOF(obj), METHOD_TO_STRING);
    if (to_string != NULL) {
        printf("%s", bytearray_to_c_str(to_string(obj, NULL)->bytearray));
    } else {
        printf("<%s>", type_names[TYPEOF(obj)]);
    }
}

static void print_list(obj_t *list_obj) {
    printf("{ ");

    obj_list_element_t *root = list_obj->list->elems;
    while (root != NULL) {
        print_value(root->node);
        root = root->next;

        if (root != NULL) {
            printf(", ");
        }
    }

    printf(" }");
}

static void print_dict(obj_t *dict_obj) {
    if (dict_obj->dict->nelems < 1) {
        printf("{}");
        return;
    }

    printf("{ ");
    obj_list_element_t *root = dict_obj_keys(dict_obj, NULL)->list->elems;
    while (root != NULL) {
        print_value(root->node);
        printf("=> ");
        print_value(dict_obj_get(dict_obj, wrap_varargs(1, root->node)));
        root = root->next;
        if (root != NULL) printf("\n  ");
    }
    printf(" }");
}

static void print_result(obj_t *obj) {
    if (TYPEOF(obj) == TYPE_LIST) {
        print_list(obj);
    } else if (TYPEOF(obj) == TYPE_DICT) {
        print_dict(obj);
    } else {
        print_value(obj);
    }
    printf("\n");
}

int main() {
    // Init ethel memory management.
    mem_init('x');

    mem_set(input, 0, MAX_INPUT);

    char *program = malloc(2);
    program[0] = 0;
    unsigned int indent = 0;

    interp_t interp;
    interp_init(&interp);

    eval_result_t *result = (eval_result_t *) alloc_type(EVAL_RESULT, F_NONE);
    put_env_gc_root(&interp, (gc_header_t *) result);

    while (1) {
        if (indent) {
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
        } else {
            printf("> ");
        }
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Bye!\n");
            goto done;
        }
        size_t program_len = c_str_len(program) + c_str_len(input) + 1;
        program = (char *) realloc(program, program_len);
        c_str_ncat(program, input, program_len);

        eval(&interp, program, result);
        obj_t *obj = (obj_t *) result->obj;
        indent = result->depth;

        if (result->err != ERR_LEX_INCOMPLETE_INPUT) {
            program = realloc(program, 2);
            program[0] = 0;
            indent = 0;

            if (obj != NULL
                && result->err == ERR_NO_ERROR
                && TYPEOF(obj) != TYPE_NOTHING) {
                print_result(obj);
            } else {
                printf("%s\n", err_names[result->err]);
            }
        }
    }

    done:
    free(program);
    program = NULL;
    return 0;
}

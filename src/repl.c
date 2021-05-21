#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/int.h"
#include "../inc/str.h"
#include "../inc/dict.h"
#include "../inc/type.h"
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define MAX_INPUT 1024

char input[MAX_INPUT] = "";

static void print_value(obj_t *obj) {
  static_method to_string = get_static_method(obj->type, METHOD_TO_STRING);
  if (to_string != NULL) {
    printf("%s", bytearray_to_c_str(to_string(obj, NULL)->bytearray));
  } else {
    printf("<%s>", obj_type_names[obj->type]);
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
    printf(": ");
    print_value(dict_obj_get(dict_obj, wrap_varargs(1, root->node)));
    root = root->next;
    if (root != NULL) printf("\n  ");
  }
  printf(" }");
}

static void print_result(obj_t *obj) {
  printf(" => ");
  if (obj->type == TYPE_LIST) {
    print_list(obj);
  } else if (obj->type == TYPE_DICT) {
    print_dict(obj);
  } else {
    print_value(obj);
  }
  printf("\n");
}

int main(int argc, char** argv) {
  mem_set(input, 0, MAX_INPUT);

  char* program = mem_alloc(2);
  program[0] = 0;
  unsigned int indent = 0;

  env_t env;
  env_init(&env);
  enter_scope(&env);

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
    program = (char*) mem_realloc(program, program_len);
    c_str_ncat(program, input, program_len);

    eval_result_t *result = eval(&env, program);
    obj_t *obj = (obj_t*)result->obj;
    indent = result->depth;

    if (result->err != ERR_LEX_INCOMPLETE_INPUT) {
      mem_free(program);
      program = mem_alloc(2);
      indent = 0;
      program[0] = 0;

      if (result->err == ERR_NO_ERROR && obj->type != TYPE_NOTHING) {
        print_result(obj);
      } else {
        printf("%s\n", err_names[result->err]);
      }
    }
  }

done:
  mem_free(program);
  return 0;
}


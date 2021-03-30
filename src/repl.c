#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define MAX_INPUT 80

char result_buf[80];
char input[MAX_INPUT] = "";

void print_value(obj_t *obj) {
  switch (obj->type) {
    case TYPE_INT:
      printf("%d", obj->intval);
      break;
    case TYPE_FLOAT:
      printf("%f", (double) obj->floatval);
      break;
    case TYPE_STRING:
      printf("\"%s\"", obj->stringval);
      break;
    case TYPE_CHAR:
      // Range check here.
      printf("'%c'", obj->charval);
      break;
    case TYPE_BOOLEAN:
      if (obj->intval) {
        printf("True");
      } else {
        printf("False");
      }
      break;
    default:
      printf("<%s>", obj_type_names[obj->type]);
  }
}

void print_list(obj_t *list_obj) {
  printf("{ ");

  obj_list_element_t *root = list_obj->list->elems;
  while(root != NULL) {
    print_value(root->node);
    root = root->next;

    if (root != NULL) {
      printf(", ");
    }
  }

  printf(" }");
}

void print_result(obj_t *obj) {
  if (obj->type == TYPE_LIST) {
    print_list(obj);
  } else {
    print_value(obj);
  }
  printf("\n");
}

int main(int argc, char** argv) {
  memset(input, 0, MAX_INPUT);

  char* program = malloc(2);
  program[0] = 0;
  unsigned int indent = 0;

  env_t env;
  env_init(&env);
  push_scope(&env);

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
    size_t program_len = strlen(program) + strlen(input) + 1;
    program = (char*) realloc(program, program_len);
    strncat(program, input, program_len);

    eval_result_t *result = eval(&env, program);
    obj_t *obj = (obj_t*)result->obj;
    indent = result->depth;

    if (result->err != LEX_INCOMPLETE_INPUT) {
      free(program);
      program = malloc(2);
      indent = 0;
      program[0] = 0;

      if (result->err == NO_ERROR && obj->type != TYPE_NOTHING) {
        print_result(obj);
      } else {
        printf("%s\n", err_names[result->err]);
      }
    }
  }

done:
  free(program);
  return 0;
}


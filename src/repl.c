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

void print_result(obj_t *obj) {
  printf("%s ", obj_type_names[obj->type]);

  switch (obj->type) {
    case TYPE_INT:
      printf("%d\n", obj->intval);
      break;
    case TYPE_FLOAT:
      printf("%f\n", (double) obj->floatval);
      break;
    case TYPE_STRING:
      printf("%s\n", obj->stringval);
      break;
    case TYPE_CHAR:
      // Range check here.
      printf("'%c'\n", obj->charval);
      break;
    case TYPE_BOOLEAN:
      if (obj->intval) {
        printf("True\n");
      } else {
        printf("False\n");
      }
      break;
    default:
      // The type was printed. That's fine.
      printf("\n");
      break;
  }
}

int main(int argc, char** argv) {
  memset(input, 0, MAX_INPUT);

  char* program = malloc(sizeof(1));
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
      return -1;
    }
    size_t program_len = strlen(program) + strlen(input) + 1;
    program = (char*) realloc(program, program_len);
    strncat(program, input, program_len);

    eval_result_t *result = eval(&env, program);
    obj_t *obj = (obj_t*)result->obj;
    indent = result->depth;

    if (result->err != LEX_INCOMPLETE_INPUT) {
      free(program);
      program = malloc(sizeof(1));
      indent = 0;
      program[0] = 0;

      if (result->err == NO_ERROR && obj->type != TYPE_NOTHING) {
        print_result(obj);
      }
      printf("%s\n", err_names[result->err]);
    }
  }

  return 0;
}


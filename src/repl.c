#include <stdio.h>
#include <string.h>
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define MAX_INPUT 80

char result_buf[80];

int main(int argc, char** argv) {
  char input[MAX_INPUT];
  memset(input, 0, MAX_INPUT);

  env_t env;
  env_init(&env);
  push_scope(&env);

  while (1) {
    printf("> ");
    if (!fgets(input, sizeof(input), stdin)) {
      printf("Bye!\n");
      return -1;
    }
    eval_result_t *result = eval(&env, input);

    if (result->err == NO_ERROR) {
      printf("%s ", obj_type_names[((obj_t*)result->obj)->type]);

      switch (((obj_t*)result->obj)->type) {
        case TYPE_INT:
          printf("%d\n", ((obj_t*)result->obj)->intval);
          break;
        case TYPE_FLOAT:
          printf("%f\n", ((obj_t*)result->obj)->floatval);
          break;
        case TYPE_STRING:
          printf("%s\n", ((obj_t*)result->obj)->stringval);
          break;
        case TYPE_CHAR:
          printf("'%c'\n", ((obj_t*)result->obj)->charval);
          break;
        case TYPE_BOOLEAN:
          if (((obj_t*)result->obj)->intval) {
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

    printf("%s\n", err_names[result->err]);
  }

  return 0;
}


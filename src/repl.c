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
    obj_t *obj = (obj_t*)result->obj;

    if (result->err == NO_ERROR && obj->type != TYPE_NOTHING) {

      printf("%s ", obj_type_names[obj->type]);

      switch (obj->type) {
        case TYPE_INT:
          printf("%d\n", obj->intval);
          break;
        case TYPE_FLOAT:
          printf("%f\n", obj->floatval);
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

    printf("%s\n", err_names[result->err]);
  }

  return 0;
}


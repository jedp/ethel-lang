#include <stdio.h>
#include <string.h>
#include "../inc/eval.h"

#define MAX_INPUT 80

int main(int argc, char** argv) {
  char input[MAX_INPUT];
  memset(input, 0, MAX_INPUT);

  while (1) {
    printf("> ");
    if (!fgets(input, sizeof(input), stdin)) {
      printf("Bye!\n");
      return -1;
    }
    eval_result_t *result = eval(input);

    if (result->err != AST_NO_ERROR) {
        printf("error %d\n", result->err);
    } else if (result->type == AST_INT) {
        printf("Int: %d\n", result->intval);
    } else if (result->type == AST_FLOAT) {
        printf("Float: %f\n", result->floatval);
    } else {
        printf("shrug!?\n");
    }
  }

  return 0;
}

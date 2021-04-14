#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/str.h"
#include "../inc/err.h"
#include "../inc/obj.h"
#include "../inc/env.h"
#include "../inc/eval.h"

#define MAX_INPUT 1024

char result_buf[80];
char input[MAX_INPUT] = "";

static char* byte_repr(char c) {
  if (c >= ' ' && c <= '~') {
    char* s = mem_alloc(4);
    s[0] = '\'';
    s[1] = c;
    s[2] = '\'';
    s[3] = '\0';
    return s;
  }

  char* s = mem_alloc(5);
  s[0] = '0';
  s[1] = 'x';
  s[2] = hex_char((c & 0xf0) >> 4);
  s[3] = hex_char(c & 0xf);
  s[4] = '\0';
  return s;
}

void print_value(obj_t *obj) {
  switch (obj->type) {
    case TYPE_FUNC_PTR:
      printf("Function");
      break;
    case TYPE_INT:
      printf("%d", obj->intval);
      break;
    case TYPE_FLOAT:
      printf("%f", (double) obj->floatval);
      break;
    case TYPE_STRING:
      printf("%s", bytearray_to_c_str(obj->bytearray));
      break;
    case TYPE_BYTEARRAY:
      printf("Byte Array");
      break;
    case TYPE_BYTE:
      // Range check here.
      printf("%s", byte_repr(obj->byteval));
      break;
    case TYPE_BOOLEAN:
      if (obj->boolval) {
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
  mem_set(input, 0, MAX_INPUT);

  char* program = mem_alloc(2);
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


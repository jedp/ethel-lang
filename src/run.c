#include <stdio.h>
#include <sys/errno.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/env.h"
#include "../inc/str.h"
#include "../inc/eval.h"
#include "../inc/run.h"

static int _eval(char* program) {
  env_t env;
  env_init(&env);
  enter_scope(&env);

  eval_result_t *result = (eval_result_t*) alloc_type(EVAL_RESULT, F_NONE);
  put_env_gc_root(&env, (gc_header_t*) result);
  enter_scope(&env);

  eval(&env, program, result);

  if (result->err != ERR_NO_ERROR) {
    printf("Error: %s\n", err_names[result->err]);
    fputs("Execution error\n", stderr);
    return result->err;
  }

  obj_t *obj = (obj_t*) result->obj;
  static_method to_string = get_static_method(TYPEOF(obj), METHOD_TO_STRING);
  if (to_string != NULL) {
    printf("=> [%s] %s\n",
      type_names[TYPEOF(obj)],
      bytearray_to_c_str(to_string(obj, NULL)->bytearray));
  }

  return ERR_NO_ERROR;
}

int run(char* fname) {
  FILE *fp = fopen(fname, "r");
  char *program = NULL;

  if (fp == NULL) {
    fputs("Read error\n", stderr);
    return errno;
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    fputs("Error determining file length\n", stderr);
    return errno;
  }

  // We're going to secretly wrap the entire program between curly braces,
  // so add another two bytes to the size.
  size_t bufsize = ftell(fp) + 2;
  if (bufsize == -1) {
    fputs("File error", stderr);
    return errno;
  }

  // One for the null, two for the enclosing braces.
  program = mem_alloc(bufsize + 3);
  program[0] = '{';
  program[bufsize - 1] = '}';
  program[bufsize] = '\0';


  if (fseek(fp, 0L, SEEK_SET) != 0) {
    fputs("Error rewinding file\n", stderr);
    return errno;
  }

  // Start writing the source right after the first brace.
  size_t f_len = fread(program + 1, sizeof(char), bufsize, fp);
  if (f_len == 0) {
    fputs("Zero bytes read\n", stderr);
    return 0;
  }
  if (ferror(fp) != 0) {
    fputs("Read error\n", stderr);
    return errno;
  }
  fclose(fp);

  return _eval(program);

  mem_free(program);
  program = NULL;
  return 0;
}

int main(int argc, char** argv) {
  /*
   * Init the ethel memory manager. This file hackily uses both stdlib's
   * heap and the heap in ethel. TODO: Fix.
   */
  mem_init('x');

  if (argc != 2) {
    fputs("Usage: run <file.e>\n", stderr);
    return -1;
  }

  char* fname = argv[1];
  return run(fname);
}

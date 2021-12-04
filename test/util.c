#include <stdarg.h>
#include <stdio.h>
#include "../inc/mem.h"
#include "util.h"

 obj_method_args_t *n_args(int n, ...) {
  va_list vargs;
  va_start(vargs, n);

  obj_method_args_t *args = mem_alloc(sizeof(obj_method_args_t));
  obj_method_args_t *root = args;

  for (int i = 0; i < n; i++) {
    int val = va_arg(vargs, int);
    args->arg = int_obj(val);

    if (i < n - 1) {
      args->next = mem_alloc(sizeof(obj_method_args_t));
    } else {
      args->next = NULL;
    }

    args = args->next;
  }

  return root;
}

obj_t *make_list(int n_elems, ...) {
  if (n_elems == 0) {
    return list_obj(NULL);
  }

  obj_list_element_t *elem = mem_alloc(sizeof(obj_list_element_t));
  obj_list_element_t *root_elem = elem;

  elem->node = NULL;
  elem->next = NULL;

  va_list vargs;
  va_start(vargs, n_elems);

  for (int i = 0; i < n_elems; i++) {
    int val = va_arg(vargs, int);
    elem->node = int_obj(val);
    if (i < n_elems - 1) {
      elem->next = mem_alloc(sizeof(obj_list_element_t));
    } else {
      elem->next = NULL;
    }

    elem = elem->next;
  }

  va_end(vargs);

  return list_obj(root_elem);
}


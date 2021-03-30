#include <stdlib.h>
#include <stdio.h>
#include "../inc/obj.h"
#include "../inc/list.h"

obj_t *list_len(obj_t *list_obj, obj_method_args_t /* ignored */ *args) {
  int len = 0;
  obj_list_element_t *root = list_obj->list->elems;

  while(root != NULL) {
    len++;
    root = root->next;
  }
  return int_obj(len);
}

obj_t *list_get(obj_t *list_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }
  // Get first arg as int offset.
  obj_t *arg = args->arg;
  if (arg->type != TYPE_INT) {
    return nil_obj();
  }
  int offset = arg->intval;
  int i = 0;
  obj_list_element_t *root = list_obj->list->elems;

  while (root != NULL) {
    if (i == offset) {
      return root->node;
    }
    i++;
    root = root->next;
  }

  if (offset > i) {
    printf("Index out of bounds\n");
    return nil_obj();
  }

  return nil_obj();
}

obj_t *list_slice(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_head(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_tail(obj_t *list_obj, obj_method_args_t *args){
  return nil_obj();
}

obj_t *list_prepend(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_append(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_remove_first(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_remove_last(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *list_remove(obj_t *list_obj, obj_method_args_t *args) {
  return nil_obj();
}

void make_list_methods(obj_t *list_obj) {
  obj_method_t *m_length = malloc(sizeof(obj_method_t));
  m_length->name = METHOD_NAME_LENGTH;
  m_length->callable = list_len;

  obj_method_t *m_get = malloc(sizeof(obj_method_t));
  m_get->name = METHOD_NAME_GET;
  m_get->callable = list_get;

  m_get->next = NULL;
  m_length->next = m_get;

  list_obj->methods = m_length;
}


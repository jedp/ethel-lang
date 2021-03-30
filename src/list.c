#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../inc/obj.h"
#include "../inc/list.h"

obj_t *_list_get(obj_t *list_obj, int offset) {
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

obj_t *_list_slice(obj_t *list_obj, int start, int end) {
  if (end != -1 && end <= start) {
    printf("Bad range for slice\n");
    return nil_obj();
  }

  int i = 0;
  obj_list_element_t *root = list_obj->list->elems;

  while(i != start) {
    if (root == NULL && i != start) {
      printf("Index out of bounds\n");
      return nil_obj();
    }

    // Advance to the start item.
    root = root->next;
    i++;
  }

  // Allocate a new list to return.
  obj_t *slice = malloc(sizeof(obj_t));
  slice->type = TYPE_LIST;
  slice->flags = F_NONE;
  slice->list = malloc(sizeof(obj_list_t));
  make_list_methods(slice);

  // Give it the same type and put the first elem in it.
  slice->list->type_name = malloc(strlen(list_obj->list->type_name) + 1);
  strcpy(slice->list->type_name, list_obj->list->type_name);
  slice->list->elems = malloc(sizeof(obj_list_t));

  obj_list_element_t *curr = slice->list->elems;
  curr->node = root->node;

  // Up to but not including end.
  i++;
  while (root->next != NULL && i != end) {
    obj_list_element_t *list_next = malloc(sizeof(obj_list_element_t));
    list_next->node = root->next->node;
    curr->next = list_next;
    curr = list_next;

    curr->next = NULL;
    root = root->next;
    i++;
  }

  curr->next = NULL;
  return slice;
}

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
  return _list_get(list_obj, offset);
}

obj_t *list_slice(obj_t *list_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to slice()\n");
    return nil_obj();
  }

  obj_t *start_arg = args->arg;
  if (start_arg->type != TYPE_INT) {
    return nil_obj();
  }

  obj_t *end_arg = args->next->arg;
  if (end_arg->type != TYPE_INT) {
    return nil_obj();
  }

  return _list_slice(list_obj, start_arg->intval, end_arg->intval);
}

obj_t *list_head(obj_t *list_obj, obj_method_args_t *args) {
  return _list_get(list_obj, 0);
}

obj_t *list_tail(obj_t *list_obj, obj_method_args_t *args){
  return _list_slice(list_obj, 1, -1);
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

  obj_method_t *m_head = malloc(sizeof(obj_method_t));
  m_head->name = METHOD_NAME_HEAD;
  m_head->callable = list_head;

  obj_method_t *m_tail = malloc(sizeof(obj_method_t));
  m_tail->name = METHOD_NAME_TAIL;
  m_tail->callable = list_tail;

  obj_method_t *m_slice = malloc(sizeof(obj_method_t));
  m_slice->name = METHOD_NAME_SLICE;
  m_slice->callable = list_slice;

  // Link methods together.
  m_length->next = m_get;
  m_get->next = m_head;
  m_head->next = m_tail;
  m_tail->next = m_slice;
  m_slice->next = NULL;

  list_obj->methods = m_length;
}


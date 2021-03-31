#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../inc/obj.h"
#include "../inc/list.h"

obj_t *_empty_list(char* type_name) {
  obj_t *obj = malloc(sizeof(obj_t));
  obj->type = TYPE_LIST;
  obj->flags = F_NONE;

  obj_list_t *list = malloc(sizeof(obj_list_t));
  list->type_name = malloc(strlen(type_name) + 1);
  strcpy(list->type_name, type_name);
  list->elems = NULL;
  obj->list = list;
  return obj;
}

obj_list_element_t *_get_elem(obj_t *list_obj, int offset) {
  int i = 0;
  obj_list_element_t *root = list_obj->list->elems;

  while (root != NULL) {
    if (i == offset) {
      return root;
    }
    i++;

    if (offset < 0) {
      printf("This should not happen. Offset is %d.\n", offset);
      return NULL;
    }

    root = root->next;
  }

  return NULL;
}

int _list_len(obj_t *list_obj) {
  int len = 0;
  obj_list_element_t *root = list_obj->list->elems;

  while(root != NULL) {
    len++;
    root = root->next;
  }
  return len;
}

obj_t *_list_get(obj_t *list_obj, int offset) {
  if (offset < 0) {
    int len = _list_len(list_obj);
    offset = offset + len;
  }

  if (offset < 0) return nil_obj();

  obj_list_element_t *elem = _get_elem(list_obj, offset);

  if (elem == NULL) return nil_obj();

  return elem->node;
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
      return _empty_list(list_obj->list->type_name);
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
  return int_obj(_list_len(list_obj));
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
  if (_list_len(list_obj) == 1) {
    return _empty_list(list_obj->list->type_name);
  }
  return _list_slice(list_obj, 1, -1);
}

obj_t *list_prepend(obj_t *list_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Argument missing\n");
    return nil_obj();
  }

  if (strcmp(obj_type_names[args->arg->type], list_obj->list->type_name)) {
    printf("Wrong type\n");
    return nil_obj();
  }

  obj_list_element_t *head = _get_elem(list_obj, 0);
  obj_list_element_t *new_elem = malloc(sizeof(obj_list_element_t));
  new_elem->node = args->arg;
  new_elem->next = head;
  list_obj->list->elems = new_elem;

  return list_obj;
}

obj_t *list_append(obj_t *list_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Argument missing\n");
    return nil_obj();
  }

  if (strcmp(obj_type_names[args->arg->type], list_obj->list->type_name)) {
    printf("Wrong type\n");
    return nil_obj();
  }

  int len = _list_len(list_obj);
  obj_list_element_t *last = _get_elem(list_obj, len - 1);
  obj_list_element_t *new_elem = malloc(sizeof(obj_list_element_t));
  new_elem->node = args->arg;
  new_elem->next = NULL;

  if (last == NULL) {
    list_obj->list->elems = new_elem;
  } else {
    last->next = new_elem;
  }

  return list_obj;
}

obj_t *list_remove_first(obj_t *list_obj, obj_method_args_t *args) {
  obj_list_element_t *head = _get_elem(list_obj, 0);

  if (head != NULL) {
    obj_list_element_t *new_head = head->next;
    list_obj->list->elems = new_head;
    return head->node;
  }
  return nil_obj();
}

obj_t *list_remove_last(obj_t *list_obj, obj_method_args_t *args) {
  obj_list_element_t *first = _get_elem(list_obj, 0);
  if (first == NULL) {
    return nil_obj();
  }

  int len = _list_len(list_obj);
  obj_list_element_t *last = _get_elem(list_obj, len - 1);
  if (first == last) {
    printf("first is last\n");
    obj_t *obj = first->node;
    list_obj->list->elems = NULL;
    free(last);
    first->node = NULL;
    first->next = NULL;
    return obj;
  }

  obj_list_element_t *new_last = _get_elem(list_obj, len - 2);
  new_last->next = NULL;
  obj_t *obj = last->node;
  free(last);
  last->node = NULL;
  last->next = NULL;
  return obj;
}

obj_t *list_remove_at(obj_t *list_obj, obj_method_args_t *args) {
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
  int len = _list_len(list_obj);

  // Convert negative offset.
  if (offset < 0) offset = len + offset;
  if (offset < 0) return nil_obj();

  // Special cases. Negative offset moves backwards from end: -1 is last elem.
  if (len == 0) return nil_obj();
  if (offset >= len) return nil_obj();
  if (offset == 0) return list_remove_first(list_obj, NULL);
  if (offset == len - 1) return list_remove_last(list_obj, NULL);

  // If it's not one of those, we can get the previous element and stitch from there.
  obj_list_element_t *prev = _get_elem(list_obj, offset - 1);
  obj_list_element_t *target = prev->next;
  obj_list_element_t *subseq = target->next;
  prev->next = subseq;

  obj_t *r = target->node;

  free(target);
  target->node = NULL;
  target->next = NULL;
  return r;
}

static_method get_list_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_LENGTH: return list_len;
    case METHOD_GET: return list_get;
    case METHOD_HEAD: return list_head;
    case METHOD_TAIL: return list_tail;
    case METHOD_SLICE: return list_slice;
    case METHOD_PREPEND: return list_prepend;
    case METHOD_APPEND: return list_append;
    case METHOD_REMOVE_FIRST: return list_remove_first;
    case METHOD_REMOVE_LAST: return list_remove_last;
    case METHOD_REMOVE_AT: return list_remove_at;
    default: return NULL;
  }
}

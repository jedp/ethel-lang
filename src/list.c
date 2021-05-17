#include <stdio.h>
#include "../inc/def.h"
#include "../inc/math.h"
#include "../inc/mem.h"
#include "../inc/obj.h"
#include "../inc/list.h"
#include "../inc/type.h"
#include "../inc/rand.h"

static obj_t *_empty_list() {
  obj_t *obj = mem_alloc(sizeof(obj_t));
  obj->type = TYPE_LIST;
  obj->flags = F_ASSIGNABLE;

  obj_list_t *list = mem_alloc(sizeof(obj_list_t));
  list->elems = NULL;
  obj->list = list;
  return obj;
}

static obj_list_element_t *_get_elem(obj_t *obj, int offset) {
  int i = 0;
  obj_list_element_t *root = obj->list->elems;

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

static int _list_len(obj_t *obj) {
  int len = 0;
  obj_list_element_t *root = obj->list->elems;

  while(root != NULL) {
    len++;
    root = root->next;
  }
  return len;
}

static obj_t *_list_get(obj_t *obj, int offset) {
  if (offset < 0) {
    int len = _list_len(obj);
    offset = offset + len;
  }

  if (offset < 0) return nil_obj();

  obj_list_element_t *elem = _get_elem(obj, offset);

  if (elem == NULL) return nil_obj();

  return elem->node;
}

obj_t *_list_slice(obj_t *obj, int start, int end) {
  if (end != -1 && end <= start) {
    printf("Bad range for slice\n");
    return nil_obj();
  }

  dim_t len = _list_len(obj);
  if (abs(start) > len || abs(end) > len) {
    return _empty_list();
  }

  int i = 0;
  obj_list_element_t *root = obj->list->elems;

  while(i != start) {
    if (root == NULL && i != start) {
      return _empty_list();
    }

    // Advance to the start item.
    root = root->next;
    i++;
  }

  // Allocate a new list to return.
  obj_t *slice = mem_alloc(sizeof(obj_t));
  slice->type = TYPE_LIST;
  slice->flags = F_NONE;
  slice->list = mem_alloc(sizeof(obj_list_t));

  slice->list->elems = mem_alloc(sizeof(obj_list_t));

  obj_list_element_t *curr = slice->list->elems;
  curr->node = root->node;

  // Up to but not including end.
  i++;
  while (root->next != NULL && i != end) {
    obj_list_element_t *list_next = mem_alloc(sizeof(obj_list_element_t));
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

obj_t *list_contains(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }

  obj_t *arg = args->arg;
  obj_list_element_t *root = obj->list->elems;
  while(root != NULL) {
    if (obj_prim_eq(root->node, arg)) {
      return boolean_obj(True);
    }
    root = root->next;
  }
  return boolean_obj(False);
}

obj_t *list_hash(obj_t *obj, obj_method_args_t /* Ignored */ *args) {
  uint32_t temp = FNV32Basis;

  for (dim_t i = 0; i < _list_len(obj); i++) {
    obj_t *val = _list_get(obj, i);
    obj_type_t t = val->type;
    uint32_t h = get_static_method(t, METHOD_HASH)(val, NULL)->intval;
    temp = FNV32Prime * (temp ^ h);
  }

  return int_obj(temp);
}

obj_t *list_eq(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *arg = args->arg;

  if (arg->type != TYPE_LIST) return boolean_obj(False);

  if (_list_len(obj) != _list_len(arg)) return boolean_obj(False);

  obj_list_element_t *root = obj->list->elems;
  obj_list_element_t *other_root = arg->list->elems;
  while(root != NULL) {
    static_method ne = get_static_method(root->node->type, METHOD_NE);
    // TODO is loosey-goosey equality correct? (int 0 eq byte 0, etc.)
    if (ne(root->node, wrap_varargs(1, other_root->node))->boolval == True) {
      printf("not equal!\n");
      return boolean_obj(False);
    }
    root = root->next;
    other_root = other_root->next;
  }
  return boolean_obj(True);
}

obj_t *list_ne(obj_t *obj, obj_method_args_t *args) {
  return boolean_obj(list_eq(obj, args)->boolval == True ? False : True);
}

obj_t *list_len(obj_t *obj, obj_method_args_t /* ignored */ *args) {
  return int_obj(_list_len(obj));
}

obj_t *list_get(obj_t *obj, obj_method_args_t *args) {
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
  return _list_get(obj, offset);
}

obj_t *list_set(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to set()\n");
    return nil_obj();
  }
  // Get first arg as int offset.
  obj_t *a = args->arg;
  if (a->type != TYPE_INT) {
    printf("Int offset required.\n");
    return nil_obj();
  }
  // Check offset is valid.
  int offset = a->intval;
  if (offset < 0 || offset > _list_len(obj) - 1) {
    printf("Out of bounds\n");
    return nil_obj();
  }

  // Get second arg as elem to put at offset i.
  obj_t *b = args->next->arg;
  if (b == NULL) {
    printf("Null arg for value.\n");
    return nil_obj();
  }

  obj_list_element_t *elem = obj->list->elems;
  for (int i = 0; i < offset; i++) {
    elem = elem->next;
  }

  elem->node = b;
  return elem->node;
}

obj_t *list_slice(obj_t *obj, obj_method_args_t *args) {
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

  return _list_slice(obj, start_arg->intval, end_arg->intval);
}

obj_t *list_head(obj_t *obj, obj_method_args_t *args) {
  return _list_get(obj, 0);
}

obj_t *list_tail(obj_t *obj, obj_method_args_t *args){
  if (_list_len(obj) == 1) {
    return _empty_list();
  }
  return _list_slice(obj, 1, -1);
}

obj_t *list_prepend(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Argument missing\n");
    return nil_obj();
  }

  obj_list_element_t *head = _get_elem(obj, 0);
  obj_list_element_t *new_elem = mem_alloc(sizeof(obj_list_element_t));
  new_elem->node = args->arg;
  new_elem->next = head;
  obj->list->elems = new_elem;

  return obj;
}

obj_t *list_append(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Argument missing\n");
    return nil_obj();
  }

  int len = _list_len(obj);
  obj_list_element_t *last = _get_elem(obj, len - 1);
  obj_list_element_t *new_elem = mem_alloc(sizeof(obj_list_element_t));
  new_elem->node = args->arg;
  new_elem->next = NULL;

  if (last == NULL) {
    obj->list->elems = new_elem;
  } else {
    last->next = new_elem;
  }

  return obj;
}

obj_t *list_remove_first(obj_t *obj, obj_method_args_t *args) {
  obj_list_element_t *head = _get_elem(obj, 0);

  if (head != NULL) {
    obj_list_element_t *new_head = head->next;
    obj->list->elems = new_head;
    return head->node;
  }
  return nil_obj();
}

obj_t *list_remove_last(obj_t *obj, obj_method_args_t *args) {
  obj_list_element_t *first = _get_elem(obj, 0);
  if (first == NULL) {
    return nil_obj();
  }

  int len = _list_len(obj);
  obj_list_element_t *last = _get_elem(obj, len - 1);
  if (first == last) {
    obj_t *obj = first->node;
    obj->list->elems = NULL;
    mem_free(last);
    first->node = NULL;
    first->next = NULL;
    return obj;
  }

  obj_list_element_t *new_last = _get_elem(obj, len - 2);
  new_last->next = NULL;
  obj_t *result = last->node;
  mem_free(last);
  last->node = NULL;
  last->next = NULL;
  return result;
}

obj_t *list_remove_at(obj_t *obj, obj_method_args_t *args) {
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
  int len = _list_len(obj);

  // Convert negative offset.
  if (offset < 0) offset = len + offset;
  if (offset < 0) return nil_obj();

  // Special cases. Negative offset moves backwards from end: -1 is last elem.
  if (len == 0) return nil_obj();
  if (offset >= len) return nil_obj();
  if (offset == 0) return list_remove_first(obj, NULL);
  if (offset == len - 1) return list_remove_last(obj, NULL);

  // If it's not one of those, we can get the previous element and stitch from there.
  obj_list_element_t *prev = _get_elem(obj, offset - 1);
  obj_list_element_t *target = prev->next;
  obj_list_element_t *subseq = target->next;
  prev->next = subseq;

  obj_t *r = target->node;

  mem_free(target);
  target->node = NULL;
  target->next = NULL;
  return r;
}

obj_t *list_random_choice(obj_t *obj, obj_method_args_t *args) {
  dim_t len = _list_len(obj);
  if (len < 1) {
    return nil_obj();
  }
  return _get_elem(obj, rand32() % len)->node;
}

static_method get_list_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_HASH: return list_hash;
    case METHOD_EQ: return list_eq;
    case METHOD_NE: return list_ne;
    case METHOD_LENGTH: return list_len;
    case METHOD_CONTAINS: return list_contains;
    case METHOD_GET: return list_get;
    case METHOD_HEAD: return list_head;
    case METHOD_TAIL: return list_tail;
    case METHOD_SLICE: return list_slice;
    case METHOD_PREPEND: return list_prepend;
    case METHOD_APPEND: return list_append;
    case METHOD_REMOVE_FIRST: return list_remove_first;
    case METHOD_REMOVE_LAST: return list_remove_last;
    case METHOD_REMOVE_AT: return list_remove_at;
    case METHOD_RANDOM_CHOICE: return list_random_choice;
    default: return NULL;
  }
}


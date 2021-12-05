#include <stdio.h>
#include "../inc/def.h"
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/type.h"
#include "../inc/list.h"
#include "../inc/dict.h"
#include "../inc/obj.h"

error_t _dict_init(obj_dict_t *dict, uint32_t buckets) {
  dict_node_t **nodes = mem_alloc(sizeof(dict_node_t*) * buckets);
  if (nodes == NULL) return ERR_OUT_OF_MEMORY;

  // Null out the allocated area. Nothing for GC to trace yet.
  mem_set(nodes, 0, sizeof(dict_node_t*) * buckets);
  dict->buckets = buckets;
  dict->nelems = 0;
  dict->nodes = nodes;
  return ERR_NO_ERROR;
}

error_t dict_init(obj_t *obj, uint32_t buckets) {
  return _dict_init(obj->dict, buckets);
}

error_t _dict_put(obj_dict_t *dict, obj_t *k, obj_t *v) {
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", type_names[k->type]);
    return ERR_NO_SUCH_METHOD;
  }
  uint32_t hv = hash_obj->intval;
  uint32_t bucket_index = hv % dict->buckets;

  obj_t *k_copy = get_static_method(k->type, METHOD_COPY)(k, NULL);
  if (k_copy->type != k->type) {
    printf("Failed to copy key.\n");
    return ERR_EVAL_UNHANDLED_OBJECT;
  }

  dict_node_t *node = dict->nodes[bucket_index];
  static_method eq = get_static_method(k->type, METHOD_EQ);

  // See if this key is already in the dictionary.
  while (node != NULL) {
    if (node->hash_val == hv &&
        node->k->type == k->type &&
        eq(node->k, wrap_varargs(1, k))) {
      // There's an existing node for this key; update the value.
      node->v = v;
      return ERR_NO_ERROR;
    }  
    node = node->next;
  }

  // Create a new node and put it in the bucket.
  dict_node_t *new_node = mem_alloc(sizeof(dict_node_t));
  mark_traceable(new_node, TYPE_DICT_DATA);
  if (!new_node) return ERR_OUT_OF_MEMORY;

  new_node->hash_val = hv;
  new_node->k = k_copy;
  new_node->v = v;
  // Put the new node at the head of the list.
  new_node->next = dict->nodes[bucket_index];
  dict->nodes[bucket_index] = new_node;
  dict->nelems++;

  return ERR_NO_ERROR;
}

static error_t dict_resize(obj_t *orig_obj) {
  // Might want to be bigger or smaller.
  dim_t new_buckets =
    DICT_INIT_BUCKETS +
    (orig_obj->dict->nelems / DICT_INIT_BUCKETS) * DICT_INIT_BUCKETS;

  if (new_buckets == orig_obj->dict->buckets) {
    return ERR_NO_ERROR;
  }

  error_t err;
  obj_dict_t *new_dict = mem_alloc(sizeof(obj_dict_t));
  mark_traceable(new_dict, TYPE_DICT);

  if ((err = _dict_init(new_dict, new_buckets)) != ERR_NO_ERROR) {
    mem_free(new_dict);
    new_dict = NULL;
    return err;
  }

  // Put all of orig's items in the new dict.
  for (dim_t i = 0; i < orig_obj->dict->buckets; i++) {
    dict_node_t *kv = orig_obj->dict->nodes[i];
    while(kv != NULL) {
      _dict_put(new_dict, kv->k, kv->v);
      kv = kv->next;
    }
  }

  obj_dict_t *old_dict = orig_obj->dict;
  orig_obj->dict = new_dict;
  mem_free(old_dict);
  old_dict = NULL;

#ifdef DEBUG
  printf("Resized dict. Now %d buckets for %d elems.\n",
      new_dict->buckets, new_dict->nelems);
#endif

  return ERR_NO_ERROR;
}

error_t dict_put(obj_t *obj, obj_t *k, obj_t *v) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return ERR_TYPE_UNUSABLE_AS_KEY;
  }

  obj_dict_t *dict = obj->dict;
  error_t err;

  if ((err = _dict_put(dict, k, v)) != ERR_NO_ERROR) {
    return err;
  }

  dict_resize(obj);

  return ERR_NO_ERROR;
}

boolean dict_contains(obj_t *dict_obj, obj_t *k) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return False;
  }

  obj_dict_t *dict = dict_obj->dict;
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", type_names[k->type]);
    return ERR_NO_SUCH_METHOD;
  }
  uint32_t hv = hash_obj->intval;
  uint32_t bucket_index = hv % dict->buckets;

  dict_node_t *node = dict->nodes[bucket_index];
  static_method eq = get_static_method(k->type, METHOD_EQ);
  while (node != NULL) {
    if (node->hash_val == hv &&
        node->k->type == k->type &&
        eq(node->k, wrap_varargs(1, k))) {
      return True;
    }
    node = node->next;
  }

  return False;
}

obj_t *dict_remove(obj_t *obj, obj_t *k) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return nil_obj();
  }

  obj_dict_t *dict = obj->dict;
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", type_names[k->type]);
    return nil_obj();
  }
  uint32_t hv = hash_obj->intval;
  uint32_t bucket_index = hv % dict->buckets;
  static_method eq = get_static_method(k->type, METHOD_EQ);

  dict_node_t *prev = NULL;
  dict_node_t *node = dict->nodes[bucket_index];
  while (node != NULL) {
    // Remove this node and splice the list together.
    if (node->hash_val == hv &&
        node->k->type == k->type &&
        eq(node->k, wrap_varargs(1, k))) {

      // Remove the node from the linked list.
      if (prev != NULL) {
        // If there was a previous node, link around this one.
        prev->next = node->next;
      } else {
        // If this was the first in the list, its next is now the head.
        dict->nodes[bucket_index] = node->next;
      }
      obj_t *v = node->v;

      dict->nelems--;
      dict_resize(obj);

      mem_free(node);
      node = NULL;

      return v;
    }
    prev = node;
    node = node->next;
  }
  
  return nil_obj();
}

obj_t *dict_get(obj_t *obj, obj_t *k) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return nil_obj();
  }

  obj_dict_t *dict = obj->dict;
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", type_names[k->type]);
    return nil_obj();
  }
  uint32_t hv = hash_obj->intval;
  uint32_t bucket_index = hv % dict->buckets;

  dict_node_t *node = dict->nodes[bucket_index];
  static_method eq = get_static_method(k->type, METHOD_EQ);
  while (node != NULL) {
    if (node->hash_val == hv &&
        k->type == node->k->type &&
        eq(k, wrap_varargs(1, node->k))) {
      return node->v;
    }

    node = node->next;
  }

  return nil_obj();
}

obj_t *dict_obj_get(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  return dict_get(obj, k);
}

obj_t *dict_obj_put(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to put()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  obj_t *v = args->next->arg;
  if (k == NULL || v == NULL) {
    printf("Two args required to put in dict.\n");
    return nil_obj();
  }
  if (dict_put(obj, k, v) != ERR_NO_ERROR) {
    return nil_obj();
  }
  return v;
}

obj_t *dict_obj_in(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  return boolean_obj(dict_contains(obj, k));
}

obj_t *dict_obj_len(obj_t *obj, obj_method_args_t *args) {
  return int_obj(obj->dict->nelems);
}

obj_t *dict_obj_keys(obj_t *obj, obj_method_args_t *args) {
  obj_t *list = list_obj(NULL);
  for (dim_t i = 0; i < obj->dict->buckets; i++) {
    dict_node_t *kv = obj->dict->nodes[i];
    while (kv != NULL) {
      list_append(list, wrap_varargs(1, kv->k));
      kv = kv->next;
    }
  }
  return list;
}

obj_t *dict_obj_remove(obj_t *obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  return dict_remove(obj, k);
}

static obj_t *iter_next(obj_iter_t *iterable) {
  switch(iterable->state) {
    case ITER_NOT_STARTED:
      iterable->state = ITER_ITERATING;
      // TODO if you're grossed out by this copy of all keys, fix it.
      obj_t *keys = dict_obj_keys(iterable->obj, NULL);
      iterable->state_obj->list = keys->list;
      // Fall through to ITERATING.

    case ITER_ITERATING: {
      obj_t *elem = list_remove_first(iterable->state_obj, NULL);
      if (elem->type == TYPE_NIL) {
        iterable->state = ITER_STOPPED;
        return nil_obj();
      }
      return elem;
    }

    case ITER_STOPPED:
      return nil_obj();

    default:
      printf("Unexpected iteration state enum! %d\n", iterable->state);
      return nil_obj();
  }
}

obj_t *dict_obj_iterator(obj_t *obj, obj_method_args_t *args) {
  obj_t *start_state = list_obj(NULL);
  return iterator_obj(obj, start_state, iter_next);
}

static_method get_dict_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_GET: return dict_obj_get;
    case METHOD_CONTAINS: return dict_obj_in;
    case METHOD_LENGTH: return dict_obj_len;
    case METHOD_KEYS: return dict_obj_keys;
    case METHOD_REMOVE_ELEM: return dict_obj_remove;
    case METHOD_ITERATOR: return dict_obj_iterator;
    default: return NULL;
  }
}


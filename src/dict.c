#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/def.h"
#include "../inc/mem.h"
#include "../inc/type.h"
#include "../inc/dict.h"
#include "../inc/obj.h"

error_t dict_init(obj_t *dict_obj, uint32_t buckets) {
  dict_node_t **nodes = mem_alloc(sizeof(dict_node_t*) * buckets);
  if (nodes == NULL) return ERR_OUT_OF_MEMORY;

  memset(nodes, 0, sizeof(dict_node_t*) * buckets);
  dict_obj->dict->buckets = buckets;
  dict_obj->dict->nelems = 0;
  dict_obj->dict->nodes = nodes;
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
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", obj_type_names[k->type]);
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
    printf("Disaster! No hash method for %s\n", obj_type_names[k->type]);
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

error_t dict_remove(obj_t *dict_obj, obj_t *k) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return ERR_TYPE_UNUSABLE_AS_KEY;
  }

  obj_dict_t *dict = dict_obj->dict;
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", obj_type_names[k->type]);
    return ERR_NO_SUCH_METHOD;
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
      if (prev) {
        prev->next = node->next;
      } else if (node->next) {
        dict->nodes[bucket_index] = node->next;
      }
      free(node);
      dict->nelems--;
      return ERR_NO_ERROR;
    }
    prev = node;
    node = node->next;
  }
  
  return ERR_NO_ERROR;
}

obj_t *dict_get(obj_t *dict_obj, obj_t *k) {
  if (k->type != TYPE_INT &&
      k->type != TYPE_FLOAT &&
      k->type != TYPE_STRING &&
      k->type != TYPE_BYTE &&
      k->type != TYPE_BOOLEAN) {
    return nil_obj();
  }

  obj_dict_t *dict = dict_obj->dict;
  obj_t *hash_obj = get_static_method(k->type, METHOD_HASH)(k, NULL);
  if (hash_obj->type == TYPE_NIL) {
    printf("Disaster! No hash method for %s\n", obj_type_names[k->type]);
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

obj_t *dict_obj_get(obj_t *dict_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to get()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  return dict_get(dict_obj, k);
}

obj_t *dict_obj_put(obj_t *dict_obj, obj_method_args_t *args) {
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
  if (dict_put(dict_obj, k, v) != ERR_NO_ERROR) {
    return nil_obj();
  }
  return v;
}

obj_t *dict_obj_in(obj_t *dict_obj, obj_method_args_t *args) {
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to contains()\n");
    return nil_obj();
  }
  obj_t *k = args->arg;
  return boolean_obj(dict_contains(dict_obj, k));
}

obj_t *dict_obj_len(obj_t *dict_obj, obj_method_args_t *args) {
  return int_obj(dict_obj->dict->nelems);
}

obj_t *dict_obj_keys(obj_t *dict_obj, obj_method_args_t *args) {
  return nil_obj();
}

obj_t *dict_obj_remove(obj_t *dict_obj, obj_method_args_t *args) {
  return nil_obj();
}

static_method get_dict_static_method(static_method_ident_t method_id) {
  switch (method_id) {
    case METHOD_GET: return dict_obj_get;
    case METHOD_CONTAINS: return dict_obj_in;
    case METHOD_LENGTH: return dict_obj_len;
    case METHOD_KEYS: return dict_obj_keys;
    case METHOD_REMOVE_AT: return dict_obj_remove;
    default: return NULL;
  }
}


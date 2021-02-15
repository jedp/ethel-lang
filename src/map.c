#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/map.h"

#define FVN32Prime 0x01000193
#define FVN32Basis 0x811C9DC5

/**
 * The FNV (Fowler, Noll, Vo) non-cryptographic hash function FNV-1a
 * for 32-bit hashes returning a 32-bit integer.
 */
bool FNV32String(const char* in, uint32_t* out) {
  uint32_t temp;
  uint8_t ch;

  if (in && out)  {
    temp = FVN32Basis;
    while((ch = *in++)) {
      temp = FVN32Prime * (temp ^ ch);
    }
    *out = temp;
    return true;
  }

  // Null input pointer.
  return false;
}

bool map_init(map_t *map, uint32_t size) {
  map_node_t **nodes = malloc(sizeof(map_node_t*) * size);
  if (nodes == 0) return false;

  memset(nodes, 0, sizeof(map_node_t*) * size);
  map->size = size;
  map->nodes = nodes;
  map->nelems = 0;
  return true;
}

bool map_put(map_t *map, char *k, char *v) {
  uint32_t hv;
  if (!FNV32String(k, &hv)) return false;
  hv %= map->size;

  map_node_t *node = map->nodes[hv];

  // There's already a bucket for this hash value.
  while (node != NULL) {
    if (!strcmp(node->k, k)) {
      // There's an existing node for this key; update the value.
      node->v = v;
      return true;
    }  
    node = node->next;
  }

  // Create a new node and put it in the bucket.
  map_node_t *new_node = malloc(sizeof(map_node_t));
  if (!new_node) return false;

  new_node->hash = hv;
  new_node->k = k;
  new_node->v = v;
  // Put the new node at the head of the list.
  new_node->next = map->nodes[hv];
  map->nodes[hv] = new_node;
  map->nelems++;

  return true;
}

bool map_contains(map_t *map, char *k) {
  uint32_t hv;
  if (!FNV32String(k, &hv)) return false;
  hv %= map->size;

  map_node_t *node = map->nodes[hv];
  while (node != NULL) {
    if (!strcmp(node->k, k)) return true;
    node = node->next;
  }

  return false;
}

bool map_remove(map_t *map, char *k) {
  uint32_t hv;
  if (!FNV32String(k, &hv)) return false;
  hv %= map->size;

  map_node_t *prev = NULL;
  map_node_t *node = map->nodes[hv];
  while (node != NULL) {
    // Remove this node and splice the list together.
    if (!strcmp(node->k, k)) {
      if (prev) {
        prev->next = node->next;
      } else if (node->next) {
	map->nodes[hv] = node->next;
      }
      free(node);
      map->nelems--;
      return true;
    }
    prev = node;
    node = node->next;
  }
  
  return false;
}

bool map_get(map_t *map, char *k, char **v) {
  uint32_t hv;
  if (!FNV32String(k, &hv)) return false;
  hv %= map->size;

  map_node_t *node = map->nodes[hv];
  while (node != NULL) {
    if (!strcmp(node->k, k)) {
      *v = node->v;
      return true;
    }

    node = node->next;
  }

  return false;
}


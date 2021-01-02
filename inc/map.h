#ifndef __HASH_H
#define __HASH_H

#include <stdbool.h>
#include <inttypes.h>

/**
 * Simple map of string->string.
 *
 * Doesn't support resizing or non-string k/v.
 */

typedef struct map_node {
  uint32_t hash;
  char *k;
  char *v;
  struct map_node *next;
} map_node_t;

typedef struct {
  uint32_t size;
  map_node_t **nodes;
  uint32_t nelems;
} map_t;

bool map_init(map_t *map, uint32_t size);
bool map_put(map_t *map, char *k, char *v);
bool map_remove(map_t *map, char *k);
bool map_contains(map_t *map, char *k);
bool map_get(map_t *map, char *k, char **v);

#endif


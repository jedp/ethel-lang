#pragma once

#include "def.h"
#include "err.h"
#include "val.h"

#define MAP_NEW_BUCKETS (8)

typedef struct map_kv_node_t {
    val_t *k;
    val_t *v;
    struct map_kv_node_t *next;
    uint32_t hash_val;
} map_kv_node_t;

typedef struct map_buckets_t {
    uint32_t nbuckets;
    uint32_t nelems;
    // Allocated to size when struct is instantiated;
    map_kv_node_t *nodes[1];
} map_buckets_t;

typedef struct map_t {
    uint32_t (*hash_func)(val_t *node);

    uint8_t (*eq_func)(val_t *node, val_t *other);

    map_buckets_t *buckets;
} map_t;

uint32_t hash_primitive(val_t *e);

uint8_t eq_primitive(val_t *a, val_t *b);

map_t *map_new(
    uint8_t nbuckets,
    uint32_t (*hash_func)(val_t *node),
    uint8_t (*eq_func)(val_t *node, val_t *other)
);

void map_free(map_t *map);

map_err_t map_put(map_t *map, val_t *k, val_t *v);

val_t *map_get(map_t *map, val_t *k);

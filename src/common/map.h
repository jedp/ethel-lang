#pragma once

#include "def.h"
#include "err.h"

#define MAP_NEW_BUCKETS (8)

typedef enum map_err {
    MAP_OK,
    MAP_NOT_FOUND,
    MAP_TOO_MANY_ITEMS,
} map_err_t;

typedef enum {
    MAP_ELEM_ERR_NO_TYPE,
    MAP_ELEM_BYTE_TYPE,
    MAP_ELEM_BOOL_TYPE,
    MAP_ELEM_CHAR_TYPE,
    MAP_ELEM_INT_TYPE,
    MAP_ELEM_UINT_TYPE,
    MAP_ELEM_FLOAT_TYPE,
    MAP_ELEM_ADDR_TYPE,
} map_elem_type_t;

typedef union {
    uint8_t byteval;
    uint8_t boolval;
    unsigned char charval;
    int intval;
    uint32_t uintval;
    float floatval;
    size_t addrval;
} map_elem;

typedef struct map_elem_t {
    map_elem_type_t type;
    map_elem elem;
} map_elem_t;

typedef struct map_kv_node_t {
    map_elem_t *k;
    map_elem_t *v;
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
    uint32_t (*hash_func)(map_elem_t *node);

    uint8_t (*eq_func)(map_elem_t *node, map_elem_t *other);

    map_buckets_t *buckets;
} map_t;

uint32_t hash_primitive(map_elem_t *e);

uint8_t eq_primitive(map_elem_t *a, map_elem_t *b);

map_t *map_new(
    uint8_t nbuckets,
    uint32_t (*hash_func)(map_elem_t *node),
    uint8_t (*eq_func)(map_elem_t *node, map_elem_t *other)
);

void map_free(map_t *map);

map_err_t map_put(map_t *map, map_elem_t *k, map_elem_t *v);

map_elem_t *map_get(map_t *map, map_elem_t *k);

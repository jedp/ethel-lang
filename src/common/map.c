#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../comp/cmem.h"
#include "map.h"

uint32_t hash_primitive(map_elem_t *e) {
    switch (e->type) {
        case MAP_ELEM_BOOL_TYPE:
            return (uint32_t) e->elem.boolval;
        case MAP_ELEM_BYTE_TYPE:
            return (uint32_t) e->elem.byteval;
        case MAP_ELEM_CHAR_TYPE:
            return (uint32_t) e->elem.charval;
        case MAP_ELEM_UINT_TYPE:
            return (uint32_t) e->elem.uintval;
        case MAP_ELEM_FLOAT_TYPE:
            return (uint32_t) e->elem.uintval;
        case MAP_ELEM_INT_TYPE:
            return (uint32_t) e->elem.intval;
        case MAP_ELEM_OBJ_TYPE:
            // TODO
            return 77;
        default:
            printf("Unsupported type %d\n", e->type);
            exit(1);
    }
}

uint8_t eq_primitive(map_elem_t *a, map_elem_t *b) {
    // TODO test b is of primitive type
    switch (a->type) {
        case MAP_ELEM_BOOL_TYPE:
            return (a->elem.boolval == b->elem.boolval) ? 1 : 0;
        case MAP_ELEM_BYTE_TYPE:
            return (a->elem.byteval == b->elem.byteval) ? 1 : 0;
        case MAP_ELEM_CHAR_TYPE:
            return (a->elem.charval == b->elem.charval) ? 1 : 0;
        case MAP_ELEM_UINT_TYPE:
        case MAP_ELEM_FLOAT_TYPE:
            return (a->elem.uintval == b->elem.uintval) ? 1 : 0;
        case MAP_ELEM_INT_TYPE:
            return (a->elem.intval == b->elem.intval) ? 1 : 0;
        case MAP_ELEM_OBJ_TYPE:
            // TODO
            return 0;
        default:
            printf("Unsupported type %d\n", a->type);
            exit(1);
    }
}

static map_buckets_t *buckets_new(uint8_t nbuckets) {
    map_buckets_t *buckets = (map_buckets_t *) comp_alloc(sizeof(map_buckets_t) * nbuckets);
    for (uint8_t i = 0; i < nbuckets; i++) {
        buckets->nodes[i] = NULL;
    }
    buckets->nelems = 0;
    buckets->nbuckets = nbuckets;
    return buckets;
}

map_t *map_new(
    uint8_t nbuckets,
    uint32_t (*hash_func)(map_elem_t *node),
    uint8_t (*eq_func)(map_elem_t *node, map_elem_t *other)
) {
    map_buckets_t *buckets = buckets_new(nbuckets);

    map_t *map = (map_t *) comp_alloc(sizeof(map_t));

    if (buckets == NULL || map == NULL) {
        printf("Can't allocate new map. Out of memory.\n");
        exit(1);
    }

    map->hash_func = hash_func;
    map->eq_func = eq_func;
    map->buckets = buckets;

    return map;
}

void map_free(map_t *map) {
    comp_free(map->buckets);
    comp_free(map);
}

static map_err_t buckets_put_internal(map_buckets_t *buckets,
                                      map_elem_t *k,
                                      map_elem_t *v,
                                      uint32_t (*hash_func)(map_elem_t *elem),
                                      uint8_t (*eq_func)(map_elem_t *node, map_elem_t *other)
) {
    uint32_t hash_val = hash_func(k);
    uint32_t bucket_index = hash_val % buckets->nbuckets;

    map_kv_node_t *first = (map_kv_node_t *) buckets->nodes[bucket_index];
    map_kv_node_t *node = first;
    while (node != NULL) {
        if (node->hash_val == hash_val &&
            eq_func(node->k, k)) {
            // Key already in the map. Update the value.
            *(node->v) = *v;
            return MAP_OK;
        }
        node = node->next;
    }

    // Create a new node.
    map_kv_node_t *new = (map_kv_node_t *) comp_alloc(sizeof(map_kv_node_t));
    if (new == NULL) {
        exit(1);
    }

    new->hash_val = hash_val;
    new->k = (map_elem_t *) comp_alloc(sizeof(map_elem_t));
    *(new->k) = *k;
    new->v = (map_elem_t *) comp_alloc(sizeof(map_elem_t));
    *(new->v) = *v;

    // Insert at head of list in this bucket.
    new->next = (first == NULL) ? NULL : first;
    buckets->nodes[bucket_index] = new;
    buckets->nelems++;

    return MAP_OK;
}

static map_err_t maybe_grow_map(map_t *orig_map) {
    uint32_t new_nbuckets =
        MAP_NEW_BUCKETS +
        (orig_map->buckets->nelems / MAP_NEW_BUCKETS) * MAP_NEW_BUCKETS;

    if (new_nbuckets == orig_map->buckets->nbuckets) {
        return MAP_OK;
    }

    map_buckets_t *new_buckets = buckets_new(new_nbuckets);

    printf("Resizing map to %d buckets...\n", new_nbuckets);

    for (uint32_t i = 0; i < orig_map->buckets->nbuckets; i++) {
        map_kv_node_t *orig_kv = orig_map->buckets->nodes[i];
        while (orig_kv != NULL) {
            buckets_put_internal(new_buckets, orig_kv->k, orig_kv->v, orig_map->hash_func, orig_map->eq_func);
            orig_kv = orig_kv->next;
        }
    }

    map_buckets_t *old_buckets = orig_map->buckets;
    orig_map->buckets = new_buckets;
    comp_free(old_buckets);

    return MAP_OK;
}

map_err_t map_put(map_t *map, map_elem_t *k, map_elem_t *v) {
    maybe_grow_map(map);

    return buckets_put_internal(map->buckets, k, v, map->hash_func, map->eq_func);
}

map_elem_t *map_get(map_t *map, map_elem_t *k) {
    uint32_t hash_val = map->hash_func(k);
    uint32_t bucket_index = hash_val % map->buckets->nbuckets;

    map_kv_node_t *node = map->buckets->nodes[bucket_index];
    while (node != NULL) {
        if (node->hash_val == hash_val &&
            map->eq_func(node->k, k)) {
            return node->v;
        }
        node = node->next;
    }

    return NULL;
}

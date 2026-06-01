#pragma once

#include <list.h>

extern int list_node_pool_init(list_node_pool_t *pool_p, unsigned int capacity);
extern void list_node_pool_fini(list_node_pool_t *pool_p);
extern int list_create_node(list_node_pool_t *pool_p, void *data);
extern int list_destroy_node(list_node_pool_t *pool_p, int node_idx);

extern int list_node_insert_before(
    list_node_pool_t   *pool_p,
    int                 target_node_idx,
    int                 node_idx
);
extern int list_node_insert_after(
    list_node_pool_t   *pool_p,
    int                 target_node_idx,
    int                 node_idx
);
extern int list_node_unlink(
    list_node_pool_t   *pool_p,
    int                 node_idx
);

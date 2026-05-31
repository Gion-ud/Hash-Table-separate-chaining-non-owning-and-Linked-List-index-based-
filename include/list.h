#pragma once

typedef struct _list_node {
    void   *data;
    int     prev_idx;
    int     next_idx;
} list_node_t;

typedef struct _list_node_pool {
    list_node_t    *node_arr;
    int            *free_list;
    int             free_head_idx;
    unsigned int    count;
    unsigned int    capacity;
} list_node_pool_t;

typedef struct _list {
    int                 head_idx;
    int                 tail_idx;
    unsigned int        length;
    list_node_pool_t   *node_pool_p;
} list_t;

#define NULL_IDX -1

extern int list_node_pool_init(list_node_pool_t *pool_p, unsigned int capacity);
extern void list_node_pool_fini(list_node_pool_t *pool_p);
extern int list_create_node(list_node_pool_t *pool_p, void *data);
extern int list_destroy_node(list_node_pool_t *pool_p, int node_idx);

#define list_node_pool_get(pool_p, idx) &(pool_p)->node_arr[idx]
#define list_node_prev_idx(pool_p, idx) (list_node_pool_get(pool_p, idx))->prev_idx
#define list_node_next_idx(pool_p, idx) (list_node_pool_get(pool_p, idx))->next_idx

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

extern int list_init(list_t *list_p, list_node_pool_t *pool_p);
extern int list_fini(list_t *list_p);

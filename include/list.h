#pragma once

typedef int node_handle_t;

typedef struct _list_node {
    void   *data;
    int     prev_idx;
    int     next_idx;
} list_node_t;

typedef list_node_t *list_iterator_t;

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

#define list_node_pool_get(pool_p, idx) &(pool_p)->node_arr[idx]
#define list_node_pool_prev_idx(pool_p, idx) (list_node_pool_get(pool_p, idx))->prev_idx
#define list_node_pool_next_idx(pool_p, idx) (list_node_pool_get(pool_p, idx))->next_idx

extern int list_init(list_t *list_p, unsigned int capacity); /* ctor */
extern void list_fini(list_t *list_p); /* dtor */

extern int list_push_front(list_t *list_p, void *data);
extern int list_push_back(list_t *list_p, void *data);
extern int list_pop_front(list_t *list_p);
extern int list_pop_back(list_t *list_p);
extern int list_get_nth_node(list_t *list_p, int idx);
extern int list_insert_before(list_t *list_p, int target_idx, void *data);
extern int list_insert_after(list_t *list_p, int target_idx, void *data);
extern int list_remove(list_t *list_p, int node_idx);


#define list_get_node list_get_node_ptr
#define list_get_nth_node_ptr(list_p, idx) \
    list_get_node(list_p, list_get_nth_node(list_p, idx))


#define list_prev_idx(list_p, node_idx) list_node_pool_prev_idx((list_p)->node_pool_p, node_idx)
#define list_next_idx(list_p, node_idx) list_node_pool_next_idx((list_p)->node_pool_p, node_idx)
#define list_get_node_ptr(list_p, node_idx) list_node_pool_get((list_p)->node_pool_p, node_idx)
#define list_head_idx(list_p) (list_p)->head_idx
#define list_tail_idx(list_p) (list_p)->tail_idx

#define list_begin(list_p) list_get_node_ptr(list_p, (list_p)->head_idx)
#define list_end(list_p) list_get_node_ptr(list_p, NULL_IDX)
#define list_next(list_p, it) \
    list_get_node_ptr(list_p, it->next_idx)

#define list_rbegin(list_p) list_get_node_ptr(list_p, (list_p)->tail_idx)
#define list_rend(list_p) list_get_node_ptr(list_p, NULL_IDX)
#define list_rnext(list_p, it) \
    list_get_node_ptr(list_p, it->prev_idx)


#include <list.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int list_node_pool_init(list_node_pool_t *pool_p, unsigned int capacity) {
    if (!pool_p || !capacity) goto failed_return;
    memset(pool_p, 0, sizeof(*pool_p));
    pool_p->node_arr = (list_node_t*)malloc(capacity * sizeof(list_node_t));
    pool_p->free_list = (int*)malloc(capacity * sizeof(int));
    if (!pool_p->node_arr || !pool_p->free_list)
        goto failed_cleanup;
    for (unsigned int i = 0; i < capacity; ++i) {
        pool_p->node_arr[i].data        = NULL;
        pool_p->node_arr[i].prev_idx    = NULL_IDX;
        pool_p->node_arr[i].next_idx    = NULL_IDX;
        pool_p->free_list[i]            = i + 1;
    }
    pool_p->free_list[capacity - 1] = NULL_IDX;

    pool_p->free_head_idx   = 0;
    pool_p->count           = 0;
    pool_p->capacity        = capacity;
    return 0;
failed_cleanup:
    list_node_pool_fini(pool_p);
failed_return:
    return -1;
}

void list_node_pool_fini(list_node_pool_t *pool_p) {
    if (!pool_p) goto scope_end;
    if (pool_p->node_arr) free(pool_p->node_arr);
    if (pool_p->free_list) free(pool_p->free_list);
    pool_p->free_head_idx   = 0;
    pool_p->count           = 0;
    pool_p->capacity        = 0;
scope_end:
    return;
}

static inline int _intrnl_list_node_pool_alloc(list_node_pool_t *pool_p) {
    if (pool_p->count == pool_p->capacity) return NULL_IDX;
    assert(pool_p->count < pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx > NULL_IDX);
    int node_idx = pool_p->free_head_idx;
    pool_p->free_head_idx = pool_p->free_list[node_idx];
    ++pool_p->count;
    return node_idx;
}
static inline int _intrnl_list_node_pool_free(list_node_pool_t *pool_p, int node_idx) {
    if (!pool_p->count) return -1;
    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx >= NULL_IDX);
    pool_p->free_list[node_idx] = pool_p->free_head_idx;
    pool_p->free_head_idx = node_idx;
    --pool_p->count;
    return 0;
}

int list_create_node(list_node_pool_t *pool_p, void *data) {
    if (
        !pool_p || !pool_p->node_arr || !pool_p->free_list ||
        pool_p->count == pool_p->capacity || !data
    )
        return NULL_IDX;
    int node_idx = _intrnl_list_node_pool_alloc(pool_p);
    if (node_idx == NULL_IDX) return -1;
    list_node_t *np = list_node_pool_get(pool_p, node_idx);
    np->data = data;
    np->prev_idx = NULL_IDX;
    np->next_idx = NULL_IDX;
    return node_idx;
}

int list_destroy_node(list_node_pool_t *pool_p, int node_idx) {
    assert(pool_p && pool_p->node_arr && pool_p->free_list);
    assert(node_idx < (int)pool_p->capacity && node_idx > NULL_IDX);
    if (
        !pool_p || !pool_p->node_arr || !pool_p->free_list ||
        node_idx >= (int)pool_p->capacity || node_idx <= NULL_IDX
    )
        return -1;

    list_node_t *np = list_node_pool_get(pool_p, node_idx);
    np->data = NULL;
    np->prev_idx = NULL_IDX;
    np->next_idx = NULL_IDX;

    return _intrnl_list_node_pool_free(pool_p, node_idx);
}

int list_node_insert_before(
    list_node_pool_t   *pool_p,
    int                 target_idx,
    int                 node_idx
) {
    if (
        !pool_p || !pool_p->node_arr || !pool_p->free_list ||
        target_idx < 0 ||
        target_idx >= (int)pool_p->capacity ||
        node_idx < 0 ||
        node_idx >= (int)pool_p->capacity ||
        target_idx == node_idx
    )
        return -1;

    int *target_prev_idx_p = &list_node_prev_idx(pool_p, target_idx);
    int *target_next_idx_p = &list_node_next_idx(pool_p, target_idx);
    int *node_prev_idx_p = &list_node_prev_idx(pool_p, node_idx);
    int *node_next_idx_p = &list_node_next_idx(pool_p, node_idx);

    assert(
        (*target_next_idx_p != NULL_IDX) ?
        list_node_prev_idx(pool_p, *target_next_idx_p) == target_idx
        : 1
    );
    assert(
        (*target_prev_idx_p != NULL_IDX) ?
        list_node_next_idx(pool_p, *target_prev_idx_p) == target_idx
        : 1
    );
    assert(
        (*node_next_idx_p != NULL_IDX) ?
        list_node_prev_idx(pool_p, *node_next_idx_p) == node_idx
        : 1
    );
    assert(
        (*node_prev_idx_p != NULL_IDX) ?
        list_node_next_idx(pool_p, *node_prev_idx_p) == node_idx
        : 1
    );

    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx >= NULL_IDX);

    list_node_prev_idx(pool_p, node_idx) = list_node_prev_idx(pool_p, target_idx);
    list_node_next_idx(pool_p, node_idx) = target_idx;

    if (
        *target_prev_idx_p != NULL_IDX
    )
        list_node_next_idx(
            pool_p,
            *target_prev_idx_p 
        ) = node_idx;

    *target_prev_idx_p  = node_idx;

    return 0;
}

int list_node_insert_after(
    list_node_pool_t   *pool_p,
    int                 target_idx,
    int                 node_idx
) {
    if (
        !pool_p || !pool_p->node_arr || !pool_p->free_list ||
        target_idx < 0 ||
        target_idx >= (int)pool_p->capacity ||
        node_idx < 0 ||
        node_idx >= (int)pool_p->capacity ||
        target_idx == node_idx
    )
        return -1;

    int *target_prev_idx_p = &list_node_prev_idx(pool_p, target_idx);
    int *target_next_idx_p = &list_node_next_idx(pool_p, target_idx);
    int *node_prev_idx_p = &list_node_prev_idx(pool_p, node_idx);
    int *node_next_idx_p = &list_node_next_idx(pool_p, node_idx);

    assert(
        (*target_next_idx_p != NULL_IDX) ?
        list_node_prev_idx(pool_p, *target_next_idx_p) == target_idx
        : 1
    );
    assert(
        (*target_prev_idx_p != NULL_IDX) ?
        list_node_next_idx(pool_p, *target_prev_idx_p) == target_idx
        : 1
    );
    assert(
        (*node_next_idx_p != NULL_IDX) ?
        list_node_prev_idx(pool_p, *node_next_idx_p) == node_idx
        : 1
    );
    assert(
        (*node_prev_idx_p != NULL_IDX) ?
        list_node_next_idx(pool_p, *node_prev_idx_p) == node_idx
        : 1
    );

    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx >= NULL_IDX);

    list_node_prev_idx(pool_p, node_idx) = target_idx;
    list_node_next_idx(pool_p, node_idx) = *target_next_idx_p;
    *target_next_idx_p = list_node_next_idx(pool_p, target_idx);

    if (
        *target_next_idx_p != NULL_IDX
    )
        list_node_prev_idx(
            pool_p,
            *target_next_idx_p
        ) = node_idx;

    *target_next_idx_p = node_idx;

    return 0;
}

int list_node_unlink(
    list_node_pool_t   *pool_p,
    int                 node_idx
) {
    if (
        !pool_p || !pool_p->node_arr || !pool_p->free_list ||
        node_idx < 0 ||
        node_idx >= (int)pool_p->capacity
    )
        return NULL_IDX;

    int *node_prev_idx_p = &list_node_prev_idx(pool_p, node_idx);
    int *node_next_idx_p = &list_node_next_idx(pool_p, node_idx);
    assert(
        (*node_next_idx_p != NULL_IDX) ?
        list_node_prev_idx(pool_p, *node_next_idx_p) == node_idx
        : 1
    );
    assert(
        (*node_prev_idx_p != NULL_IDX) ?
        list_node_next_idx(pool_p, *node_prev_idx_p) == node_idx
        : 1
    );

    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx >= NULL_IDX);

    if (*node_prev_idx_p != NULL_IDX)
        list_node_next_idx(pool_p, *node_prev_idx_p) = *node_next_idx_p;
    if (*node_next_idx_p != NULL_IDX)
        list_node_prev_idx(pool_p, *node_next_idx_p) = *node_prev_idx_p;

    return node_idx;
}

int list_init(list_t *list_p, list_node_pool_t *pool_p) {
    if (
        !list_p ||
        !pool_p || !pool_p->node_arr || !pool_p->free_list
    ) return -1;
    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_idx <= (int)pool_p->count);
    assert(pool_p->free_head_idx >= NULL_IDX);

    list_p->head_idx    = NULL_IDX;
    list_p->tail_idx    = NULL_IDX;
    list_p->length      = 0;
    list_p->node_pool_p = pool_p;

    return 0;
}

void list_fini(list_t *list_p) {
    if (!list_p) return;
    list_p->head_idx    = NULL_IDX;
    list_p->tail_idx    = NULL_IDX;
    list_p->length      = 0;
    list_p->node_pool_p = NULL;
}


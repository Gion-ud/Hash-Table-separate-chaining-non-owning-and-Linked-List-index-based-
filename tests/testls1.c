#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define NODE_POOL_CAP 32

const char *keys[] = {
    "node_0",
    "node_1",
    "node_2",
    "node_3",
    "node_4",
    "node_5",
    "node_6",
    "node_7",
    "node_8",
    "node_9",
    "node_10",
    "node_11",
    "node_12",
    "node_13",
    "node_14",
    "node_15",
};
const size_t keyc = sizeof(keys) / sizeof(*keys);

int main(void) {
    list_node_pool_t pool = {0};
    int ret = list_node_pool_init(&pool, NODE_POOL_CAP);
    assert(ret >= 0);

    puts("create node");
    int node_tail = list_create_node(&pool, (void*)keys[0]);
    assert(node_tail != NULL_IDX);
    int node_head = node_tail;
    puts("node created");
    for (size_t i = 1; i != keyc; ++i) {
        puts("it");
        int node_next = list_create_node(&pool, (void*)keys[i]);
        assert(node_next != NULL_IDX);
        list_node_insert_after(&pool, node_tail, node_next);
        node_tail = node_next;
    }

    puts("");
    for (int i = node_tail; i != NULL_IDX; i = list_node_pool_prev_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    puts("");
    for (int i = node_head; i != NULL_IDX; i = list_node_pool_next_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    puts("");
    for (size_t i = 1; i != keyc; ++i) {
        puts("it");
        int node_prev = list_create_node(&pool, (void*)keys[i]);
        assert(node_prev != NULL_IDX);
        list_node_insert_before(&pool, node_head, node_prev);
        node_head = node_prev;
    }

    puts("");
    for (int i = node_head; i != NULL_IDX; i = list_node_pool_next_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    puts("");
    for (size_t i = 1; i != keyc; ++i) {
        puts("it");
        int old_tail = list_node_unlink(&pool, node_tail);
        node_tail = list_node_pool_prev_idx(&pool, old_tail);
        int ret = list_destroy_node(&pool, old_tail);
        assert(ret != -1);
    }

    puts("");
    for (int i = node_head; i != NULL_IDX; i = list_node_pool_next_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    puts("");
    for (ptrdiff_t i = keyc - 2; i > -1; --i) {
        puts("it");
        int node_prev = list_create_node(&pool, (void*)keys[i]);
        assert(node_prev != NULL_IDX);
        list_node_insert_before(&pool, node_head, node_prev);
        node_head = node_prev;
    }


    puts("");
    for (int i = node_head; i != NULL_IDX; i = list_node_pool_next_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    for (size_t i = 1; i != keyc; ++i) {
        puts("it");
        int old_head = list_node_unlink(&pool, node_head);
        node_head = list_node_pool_next_idx(&pool, old_head);
        int ret = list_destroy_node(&pool, old_head);
        assert(ret != -1);
    }

    puts("");
    for (int i = node_head; i != NULL_IDX; i = list_node_pool_next_idx(&pool, i)) {
        list_node_t *np = list_node_pool_get(&pool, i);
        printf("%s\n", (char*)np->data);
    }

    printf("%u\n", pool.count);


    list_node_pool_fini(&pool);
    return 0;
}
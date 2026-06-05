#include <list.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define LIST_CAPACITY 32u

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

int main() {
    list_t ls = {0};
    if (list_init(&ls, LIST_CAPACITY) < 0) goto failed_return;

    for (unsigned int i = 0; i < keyc; ++i) {
        puts("list_push_back it");
        int ret = list_push_back(&ls, (void*)keys[i]);
        assert(ret >= 0);
    }
    list_iterator_t it = {0};
    puts("forward it");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    puts("backwards it");
    for (it = list_rbegin(&ls); it != list_rend(&ls); it = list_rnext(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }

    printf("\nkey: %s\n", (char*)(list_get_node_ptr(&ls, list_get_nth_node(&ls, 3)))->data);
    list_remove(&ls, list_get_nth_node(&ls, 3));
    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    puts("");
    list_insert_before(&ls, list_get_nth_node(&ls, 3), (void*)keys[3]);
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    list_insert_after(&ls, list_get_nth_node(&ls, 7), "67676767");
    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    list_remove(&ls, list_get_nth_node(&ls, 7 + 1));
    puts("");
    list_remove(&ls, list_get_nth_node(&ls, 0));
    list_remove(&ls, list_get_nth_node(&ls, 13));
    list_remove(&ls, list_get_nth_node(&ls, 6));
    list_insert_after(&ls, list_get_nth_node(&ls, 7), "67676767");
    list_insert_after(&ls, list_get_nth_node(&ls, 9), "69696969");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: [%d] %s\n", it - list_begin(&ls), (char*)it->data);
    }

    list_fini(&ls);
    return 0;
failed_return:
    return -1;
}
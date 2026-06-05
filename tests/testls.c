#include <list.h>
#include <stdlib.h>
#include <string.h>
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
        int ret = list_push_back(&ls, (void*)keys[i]);
        assert(ret >= 0);
    }
    list_iterator_t it = {0};

    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }

    list_erase_nth_node(&ls, 7);

    puts("");
    for (it = list_rbegin(&ls); it != list_rend(&ls); it = list_rnext(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    list_push_back(&ls, "node_16");
    puts("");
    for (it = list_rbegin(&ls); it != list_rend(&ls); it = list_rnext(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }

    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        if (strcmp((char*)it->data, "node_5") == 0)
            list_remove(&ls, it - list_begin(&ls));
    }

    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }

    puts("");
    for (__auto_type i = 0u; i < list_length(&ls); ++i) {
        it = list_get_nth_node_ptr(&ls, i);
        printf("key: %s\n", (char*)it->data);
    }

    for (int i = 0; i < 5; ++i) {
        list_pop_back(&ls);
        list_pop_front(&ls);
    }
    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }
    for (int i = 0; i < 5; ++i) {
        list_push_back(&ls, (void*)keys[i]);
        list_push_front(&ls, (void*)keys[keyc - i - 1]);
    }
    puts("");
    for (it = list_begin(&ls); it != list_end(&ls); it = list_next(&ls, it)) {
        printf("key: %s\n", (char*)it->data);
    }

    list_fini(&ls);
    return 0;
failed_return:
    return -1;
}
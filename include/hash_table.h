#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _hash_slot {
    const char     *key;
    unsigned int    key_len;
    uint32_t        hash;
    void           *data;
    int             prev_idx;
    int             next_idx;
} hash_slot_t;

typedef struct _hash_table {
    hash_slot_t    *slot_arr;
    int            *slot_free_list;
    int            *bucket_arr;
    int             free_head_idx;
    unsigned int    slot_count;
    unsigned int    slot_capacity;
    unsigned int    bucket_count;
    unsigned int    bucket_capacity;
} hash_table_t;


#include <stdlib.h>
#include <string.h>

extern hash_table_t *create_hash_table(
    unsigned int bucket_capacity, 
    unsigned int slot_capacity
);
extern void destroy_hash_table(hash_table_t *ht_p);

hash_table_t *
create_hash_table(
    unsigned int bucket_capacity, 
    unsigned int slot_capacity
) {
    hash_table_t *ht_p = (hash_table_t*)malloc(sizeof(hash_table_t));
    if (!ht_p) goto failed;
    memset(ht_p, 0, sizeof(*ht_p)); /* Very important: zero init */

    ht_p->slot_arr = (hash_slot_t*)calloc(slot_capacity, sizeof(*ht_p->slot_arr));
    ht_p->slot_free_list = (int*)malloc(slot_capacity * sizeof(*ht_p->slot_free_list));
    ht_p->bucket_arr = (int*)malloc(bucket_capacity * sizeof(*ht_p->bucket_arr));
    if (!ht_p->slot_arr || !ht_p->bucket_arr || !ht_p->slot_free_list)
        goto failed;
    memset(ht_p->bucket_arr, 0xFF, sizeof(*ht_p->bucket_arr));  // 0xFFFFFFFF == (int32_t)-1
    memset(ht_p->slot_free_list, 0xFF, sizeof(*ht_p->slot_free_list));

    ht_p->free_head_idx     = 0;
    ht_p->slot_count        = 0;
    ht_p->slot_capacity     = slot_capacity;
    ht_p->bucket_count      = 0;
    ht_p->bucket_capacity   = bucket_capacity;

    return ht_p;
failed:
    destroy_hash_table(ht_p); /* NULL ptr safe */
    return NULL;
}

void destroy_hash_table(hash_table_t *ht_p) {
    if (!ht_p) goto scope_end;
    if (ht_p->slot_arr) free(ht_p->slot_arr);
    if (ht_p->slot_free_list) free(ht_p->slot_free_list);
    if (ht_p->bucket_arr) free(ht_p->bucket_arr);
    free(ht_p);
scope_end:
    return;
}
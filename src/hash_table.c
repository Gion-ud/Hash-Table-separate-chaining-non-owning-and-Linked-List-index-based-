#include <hash_table.h>

#define HT_MIN_NSLOT 32
#define HT_MIN_NBUCKET 32

struct _hash_table {
    hash_slot_t    *slot_arr;
    unsigned char  *cntl_arr;
    int            *slot_free_list;
    int            *bucket_arr;
    int             free_head_idx;
    unsigned int    slot_count;
    unsigned int    slot_capacity;
    unsigned int    bucket_count;
};

#define SLOT_OCCUPIED   1
#define SLOT_EMPTY      0

#include <stdlib.h>
#include <string.h>
#include <assert.h>

hash_table_t *
create_hash_table(
    unsigned int bucket_count, 
    unsigned int slot_capacity
) {
    if (bucket_count < HT_MIN_NBUCKET) bucket_count = HT_MIN_NBUCKET;
    if (slot_capacity < HT_MIN_NSLOT) slot_capacity = HT_MIN_NSLOT;

    hash_table_t *ht_p = (hash_table_t*)malloc(sizeof(hash_table_t));
    if (!ht_p) goto failed;

    memset(ht_p, 0, sizeof(*ht_p)); /* Very important: zero init */

    ht_p->slot_arr          = (hash_slot_t*)malloc(slot_capacity * sizeof(*ht_p->slot_arr));
    ht_p->slot_free_list    = (int*)malloc(slot_capacity * sizeof(*ht_p->slot_free_list));
    ht_p->cntl_arr          = (unsigned char*)malloc(slot_capacity * sizeof(*ht_p->cntl_arr));
    ht_p->bucket_arr        = (int*)malloc(bucket_count * sizeof(*ht_p->bucket_arr));
    if (!ht_p->slot_arr || !ht_p->bucket_arr || !ht_p->slot_free_list || ht_p->cntl_arr)
        goto failed;

    memset(ht_p->bucket_arr, 0xFF, sizeof(*ht_p->bucket_arr));  // 0xFFFFFFFF == (int32_t)-1

    for (unsigned int i = 0u; i < slot_capacity; ++i) {
        ht_p->slot_arr[i].key       = NULL;
        ht_p->slot_arr[i].key_len   = 0;
        ht_p->slot_arr[i].hash      = 0;
        ht_p->slot_arr[i].data      = NULL;
        ht_p->slot_arr[i].prev_idx  = NULL_IDX;
        ht_p->slot_arr[i].next_idx  = NULL_IDX;
        ht_p->slot_free_list[i]     = i + 1;
        ht_p->cntl_arr[i]           = SLOT_EMPTY;
    }
    ht_p->slot_free_list[slot_capacity - 1] = NULL_IDX;

    ht_p->free_head_idx     = 0;
    ht_p->slot_count        = 0;
    ht_p->slot_capacity     = slot_capacity;
    ht_p->bucket_count      = bucket_count;

    return ht_p;
failed:
    destroy_hash_table(ht_p); /* NULL ptr safe */
    return NULL;
}

void destroy_hash_table(hash_table_t *ht_p) {
    if (!ht_p) goto scope_end;
    if (ht_p->slot_arr) free(ht_p->slot_arr);
    if (ht_p->cntl_arr) free(ht_p->cntl_arr);
    if (ht_p->slot_free_list) free(ht_p->slot_free_list);
    if (ht_p->bucket_arr) free(ht_p->bucket_arr);
    free(ht_p);
scope_end:
    return;
}

#define ht_get_slot(ht_p, slot_idx) \
    &(ht_p)->slot_arr[slot_idx]

#define ht_prev_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].prev_idx

#define ht_next_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].next_idx

int hash_table_insert(
    hash_table_t   *ht_p,
    hash_key_t     *key_p,
    const void     *data
) {
    if (
        !ht_p || !ht_p->slot_arr || !ht_p->cntl_arr ||
        !ht_p->slot_free_list || !ht_p->bucket_arr ||
        ht_p->slot_count == ht_p->slot_capacity ||
        !key_p ||
        !key_p->key ||
        !key_p->key_len
    ) return NULL_IDX;

    assert(ht_p->slot_count <= ht_p->slot_capacity);
    assert(ht_p->free_head_idx != NULL_IDX);
    assert(ht_p->slot_capacity >= HT_MIN_NSLOT);
    assert(ht_p->bucket_count >= HT_MIN_NBUCKET);

    int *head_p = &ht_p->bucket_arr[key_p->hash % ht_p->bucket_count];
    int *head_prev_p = &ht_prev_slot(ht_p, *head_p);

    int new_slot = ht_p->free_head_idx;
    assert(ht_p->cntl_arr[new_slot] != SLOT_OCCUPIED);

    ht_p->free_head_idx = ht_p->slot_free_list[new_slot];
    ht_p->cntl_arr[new_slot] = SLOT_OCCUPIED;

    hash_slot_t *slot_p = ht_get_slot(ht_p, new_slot);

    slot_p->key         = key_p->key;       // non owning
    slot_p->key_len     = key_p->key_len;
    slot_p->hash        = key_p->hash;
    slot_p->data        = data;             // non owning
    slot_p->prev_idx    = NULL_IDX;
    slot_p->next_idx    = *head_p;


    if (*head_p != NULL_IDX)
        *head_prev_p = new_slot;
    *head_p = new_slot;

    return new_slot;
}

int hash_table_lookup(
    const hash_table_t *ht_p,
    hash_key_t         *key_p,
    hash_slot_handle_t *out_slot_handle_p
) {
    if (
        !ht_p || !ht_p->slot_arr || !ht_p->cntl_arr ||
        !ht_p->slot_free_list || !ht_p->bucket_arr ||
        !out_slot_handle_p || ht_p->slot_count ||
        !key_p ||
        !key_p->key ||
        !key_p->key_len
    ) return NULL_IDX;

    assert(ht_p->slot_count <= ht_p->slot_capacity);
    assert(ht_p->slot_capacity >= HT_MIN_NSLOT);
    assert(ht_p->bucket_count >= HT_MIN_NBUCKET);

    out_slot_handle_p->bucket_idx = NULL_IDX;
    out_slot_handle_p->slot_idx = NULL_IDX;

    int bucket = ht_p->bucket_arr[key_p->hash % ht_p->bucket_count];
    int slot_idx = bucket;
    unsigned int probc = 0;
    while (probc < ht_p->slot_capacity && slot_idx != NULL_IDX) {
        hash_slot_t *slot_p = ht_get_slot(ht_p, slot_idx);
        if (
            ht_p->cntl_arr[slot_idx] != SLOT_OCCUPIED ||
            slot_p->hash != key_p->hash ||
            slot_p->key_len != key_p->key_len
        )
            goto probe_next_slot;
        if (memcmp(slot_p->key, key_p->key, key_p->key_len) == 0) {
            out_slot_handle_p->bucket_idx = bucket;
            out_slot_handle_p->slot_idx = slot_idx;
        }
    probe_next_slot:
        ++probc;
    }

    return NULL_IDX;
}

int hash_table_destroy_handle(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
) {
    (void)ht_p;
    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
    return 0;
}

int hash_table_remove(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
) {
    if (
        !ht_p || !ht_p->slot_arr || !ht_p->cntl_arr ||
        !ht_p->slot_free_list || !ht_p->bucket_arr ||
        !ht_p->slot_count ||
        !slot_handle_p ||
        slot_handle_p->slot_idx < 0 ||
        slot_handle_p->slot_idx >= (int)ht_p->slot_capacity ||
        slot_handle_p->bucket_idx < 0 ||
        slot_handle_p->bucket_idx >= (int)ht_p->bucket_count ||
        ht_p->cntl_arr[slot_handle_p->slot_idx] != SLOT_OCCUPIED
    ) return -1;

    assert(ht_p->slot_count <= ht_p->slot_capacity);
    assert(ht_p->slot_capacity >= HT_MIN_NSLOT);
    assert(ht_p->bucket_count >= HT_MIN_NBUCKET);

    int *prev_idx_p = ht_prev_slot(ht_p, slot_handle_p->slot_idx);
    int *next_idx_p = ht_next_slot(ht_p, slot_handle_p->slot_idx);
    /* assert linked list validity here */
    assert(
        (*prev_idx_p != NULL_IDX) ?
        ht_next_slot(ht_p, *prev_idx_p) == slot_handle_p->slot_idx
        : 1
    );
    assert(
        (*next_idx_p != NULL_IDX) ?
        ht_prev_slot(ht_p, *next_idx_p) == slot_handle_p->slot_idx
        : 1
    );

    /* unlink */
    if (*prev_idx_p != NULL_IDX)
        ht_next_slot(ht_p, *prev_idx_p) = *next_idx_p;
    else
        ht_p->bucket_arr[slot_handle_p->bucket_idx] = *next_idx_p;

    if (*next_idx_p != NULL_IDX)
        ht_prev_slot(ht_p, *next_idx_p) = *prev_idx_p;

    ht_p->slot_free_list[slot_handle_p->slot_idx] = ht_p->free_head_idx;
    ht_p->free_head_idx = slot_handle_p->slot_idx;

    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
    --ht_p->slot_count;

    return 0;
}

const hash_slot_t *hash_table_get_slot(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *slot_handle_p
) {
    if (
        !ht_p || !ht_p->slot_arr || !ht_p->cntl_arr ||
        !ht_p->slot_free_list || !ht_p->bucket_arr ||
        !ht_p->slot_count ||
        !slot_handle_p ||
        slot_handle_p->slot_idx < 0 ||
        slot_handle_p->slot_idx >= (int)ht_p->slot_capacity ||
        slot_handle_p->bucket_idx < 0 ||
        slot_handle_p->bucket_idx >= (int)ht_p->bucket_count ||
        ht_p->cntl_arr[slot_handle_p->slot_idx] != SLOT_OCCUPIED
    ) return NULL;

    assert(ht_p->slot_count <= ht_p->slot_capacity);
    assert(ht_p->slot_capacity >= HT_MIN_NSLOT);
    assert(ht_p->bucket_count >= HT_MIN_NBUCKET);

    int *prev_idx_p = ht_prev_slot(ht_p, slot_handle_p->slot_idx);
    int *next_idx_p = ht_next_slot(ht_p, slot_handle_p->slot_idx);
    /* assert linked list validity here */
    assert(
        (*prev_idx_p != NULL_IDX) ?
        ht_next_slot(ht_p, *prev_idx_p) == slot_handle_p->slot_idx
        : 1
    );
    assert(
        (*next_idx_p != NULL_IDX) ?
        ht_prev_slot(ht_p, *next_idx_p) == slot_handle_p->slot_idx
        : 1
    );

    return ht_get_slot(ht_p, slot_handle_p->slot_idx);
}



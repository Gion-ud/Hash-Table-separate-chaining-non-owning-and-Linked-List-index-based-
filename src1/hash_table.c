/*
 * Implm of now-owning hash table with
 * stable slot idx and double libked list chaining
 */

#define _HASH_TABLE_INTRNL_IMPLM
#include <hash_table.h>

struct _hash_table {
    hash_slot_t    *slot_arr;
    unsigned char  *cntl_arr;
    int            *slot_free_list;
    int            *bucket_arr;     /* arr of chain head idx */
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
#include "dbg_print.h"

hash_table_t *
create_hash_table(
    unsigned int bucket_count, 
    unsigned int slot_capacity
) {
    if (bucket_count < HT_MIN_NBUCKET) bucket_count = HT_MIN_NBUCKET;
    if (slot_capacity < HT_MIN_NSLOT) slot_capacity = HT_MIN_NSLOT;

    hash_table_t *ht_p = (hash_table_t*)malloc(sizeof(hash_table_t));
    _dbg_print("create_hash_table@0");
    assert(ht_p);
    if (!ht_p) goto failed;

    memset(ht_p, 0, sizeof(*ht_p)); /* Very important: zero init */

    ht_p->slot_arr          = (hash_slot_t*)malloc(slot_capacity * sizeof(*ht_p->slot_arr));
    _dbg_print("create_hash_table@1.0");
    assert(ht_p->slot_arr);
    ht_p->slot_free_list    = (int*)malloc(slot_capacity * sizeof(*ht_p->slot_free_list));
    _dbg_print("create_hash_table@1.1");
    assert(ht_p->slot_free_list);
    ht_p->cntl_arr          = (unsigned char*)malloc(slot_capacity * sizeof(*ht_p->cntl_arr));
    _dbg_print("create_hash_table@1.2");
    assert(ht_p->cntl_arr);
    ht_p->bucket_arr        = (int*)malloc(bucket_count * sizeof(*ht_p->bucket_arr));
    _dbg_print("create_hash_table@1.3");
    assert(ht_p->bucket_arr);
    if (!ht_p->slot_arr || !ht_p->bucket_arr || !ht_p->slot_free_list || !ht_p->cntl_arr)
        goto failed;

    memset(ht_p->bucket_arr, 0xFF, sizeof(*ht_p->bucket_arr) * bucket_count);  // 0xFFFFFFFF == (int32_t)-1

    _dbg_print("create_hash_table@2");
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

    _dbg_print("create_hash_table@3");
    ht_p->free_head_idx     = 0;
    ht_p->slot_count        = 0;
    ht_p->slot_capacity     = slot_capacity;
    ht_p->bucket_count      = bucket_count;

    _dbg_print("create_hash_table@0.ret\n");
    return ht_p;
failed:
    _dbg_print("create_hash_table@-1");
    destroy_hash_table(ht_p); /* NULL ptr safe */
    _dbg_print("create_hash_table@-1.ret\n");
    return NULL;
}

void destroy_hash_table(hash_table_t *ht_p) {
    _dbg_print("destroy_hash_table@0");
    if (!ht_p) goto scope_end;
    _dbg_print("destroy_hash_table@1");
    if (ht_p->slot_arr) free(ht_p->slot_arr);
    _dbg_print("destroy_hash_table@2");
    if (ht_p->cntl_arr) free(ht_p->cntl_arr);
    _dbg_print("destroy_hash_table@3");
    if (ht_p->slot_free_list) free(ht_p->slot_free_list);
    _dbg_print("destroy_hash_table@4");
    if (ht_p->bucket_arr) free(ht_p->bucket_arr);
    _dbg_print("destroy_hash_table@5");
    free(ht_p);
scope_end:
    _dbg_print("destroy_hash_table@ret\n");
    return;
}

#define ht_get_slot(ht_p, slot_idx) \
    &(ht_p)->slot_arr[slot_idx]

#define ht_prev_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].prev_idx

#define ht_next_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].next_idx


static inline int _is_valid_storage_ht(const hash_table_t *ht_p) {
    return (
        ht_p && ht_p->slot_arr && ht_p->cntl_arr &&
        ht_p->slot_free_list && ht_p->bucket_arr
    );
}
static inline void _ht_assert_intrnl_state(const hash_table_t *ht_p) {
    // assert(ht_p->slot_count <= ht_p->bucket_count);
    assert(ht_p->slot_count <= ht_p->slot_capacity);
    assert(ht_p->slot_capacity >= HT_MIN_NSLOT);
    assert(ht_p->bucket_count >= HT_MIN_NBUCKET);
    if (ht_p->slot_count == ht_p->slot_capacity) {
        assert(ht_p->free_head_idx == NULL_IDX);
    } else if (!ht_p->slot_count) {
        assert(ht_p->free_head_idx != NULL_IDX);
        assert(ht_p->cntl_arr[ht_p->free_head_idx] == SLOT_EMPTY);
    }
}
static inline int _is_valid_ht_slot_handle(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *hs_handle_p
) {
    return (
        hs_handle_p &&
        hs_handle_p->bucket_idx > NULL_IDX && 
        hs_handle_p->bucket_idx < (int)ht_p->bucket_count &&
        hs_handle_p->slot_idx > NULL_IDX &&
        hs_handle_p->slot_idx < (int)ht_p->slot_capacity
    );
}
static inline int _is_slot_occupied(
    const hash_table_t *ht_p,
    int                 slot_idx     
) {
    return (
        slot_idx != NULL_IDX &&
        ht_p->cntl_arr[slot_idx] == SLOT_OCCUPIED
    );
}
#define _is_valid_hash_key(hk_p) ((hk_p) && (hk_p)->key && (hk_p)->key_len)
#define _is_empty_ht(ht_p) (!(ht_p)->slot_count)
#define _is_not_empty_ht(ht_p) (!_is_empty_ht(ht_p))
static inline int _is_full_ht(const hash_table_t *ht_p) {
    return (ht_p->slot_count >= ht_p->slot_capacity);
}
#define _ht_has_rem_capacity(ht_p) (!_is_full_ht(ht_p))

static inline void
_ht_assert_slot_chain_integrity(
    const hash_table_t *ht_p,
    int                 slot_idx,
    int                 prev_idx,
    int                 next_idx
) {
    assert(
        (prev_idx != NULL_IDX) ?
        ht_next_slot(ht_p, prev_idx) == slot_idx
        : 1
    );
    assert(
        (next_idx != NULL_IDX) ?
        ht_prev_slot(ht_p, next_idx) == slot_idx
        : 1
    );
}

int hash_table_insert(
    hash_table_t       *ht_p,
    const hash_key_t   *key_p,
    const void         *data
) {
    _dbg_print("hash_table_insert@0._is_valid_storage_ht");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_insert@0.not._is_full_ht");
    if (_is_full_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_insert@0._is_valid_hash_key");
    if (!_is_valid_hash_key(key_p)) goto failed_ret;

    _dbg_print("hash_table_insert@1._ht_assert_intrnl_state");
    _ht_assert_intrnl_state(ht_p);

    _dbg_print("hash_table_insert@2.0 # head_idx");
    int *head_idx_p = &ht_p->bucket_arr[key_p->hash % ht_p->bucket_count];
    _dbg_print("hash_table_insert@2.1 # head_prev_idx");
    int *head_prev_idx_p = &ht_prev_slot(ht_p, *head_idx_p);

    _dbg_print("hash_table_insert@3.alloc_slot");
    int new_slot = ht_p->free_head_idx;
    ht_p->free_head_idx = ht_p->slot_free_list[new_slot];
    ht_p->cntl_arr[new_slot] = SLOT_OCCUPIED;

    _dbg_print("hash_table_insert@4.ht_get_slot");
    hash_slot_t *slot_p = ht_get_slot(ht_p, new_slot);
    
    _dbg_print("hash_table_insert@4.deref_slot");
    slot_p->key         = key_p->key;       // non owning
    slot_p->key_len     = key_p->key_len;
    slot_p->hash        = key_p->hash;
    slot_p->data        = data;             // non owning
    slot_p->prev_idx    = NULL_IDX;
    slot_p->next_idx    = *head_idx_p;

    _dbg_print(
        "hash_table_insert@4 # slot%d:\t('%.*s', %u, 0x%.8x, %p, %d, %d)",
        new_slot,
        slot_p->key_len, slot_p->key,
        slot_p->key_len,
        slot_p->hash,
        slot_p->data,
        slot_p->prev_idx,
        slot_p->next_idx
    );

    _dbg_print("hash_table_insert@5.chain_push_front");
    if (*head_idx_p != NULL_IDX)
        *head_prev_idx_p = new_slot;
    *head_idx_p = new_slot;

    _dbg_print("hash_table_insert@6.inc_slot_count");
    ++ht_p->slot_count;

    if (*head_idx_p != NULL_IDX) {
        _dbg_print("hash_table_insert@7._ht_assert_slot_chain_integrity");
        int slot_idx = *head_idx_p;
        int prev_idx = ht_prev_slot(ht_p, slot_idx);
        int next_idx = ht_next_slot(ht_p, slot_idx);
        _ht_assert_slot_chain_integrity(ht_p, slot_idx, prev_idx, next_idx);
    }

    _dbg_print("hash_table_insert@0.ret\n");
    return new_slot;
failed_ret:
    return NULL_IDX;
}

int hash_table_lookup(
    const hash_table_t *ht_p,
    const hash_key_t   *key_p,
    hash_slot_handle_t *out_slot_handle_p
) {
    _dbg_print("hash_table_lookup@0._is_valid_storage_ht");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_lookup@0.not._is_empty_ht");
    if (_is_empty_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_lookup@0._is_valid_hash_key");
    if (!_is_valid_hash_key(key_p)) goto failed_ret;

    _dbg_print("hash_table_lookup@0._is_valid_out_slot_handle_p");
    if (!out_slot_handle_p) goto failed_ret;

    _dbg_print("hash_table_lookup@1._ht_assert_intrnl_state");
    _ht_assert_intrnl_state(ht_p);

    out_slot_handle_p->bucket_idx = NULL_IDX;
    out_slot_handle_p->slot_idx = NULL_IDX;

    int bucket_idx = key_p->hash % ht_p->bucket_count;
    int slot_idx = ht_p->bucket_arr[bucket_idx];
    _dbg_print("hash_table_lookup@2 # bucket_idx=%d; chain_head_idx=%d", bucket_idx, slot_idx);

    _dbg_print("hash_table_lookup@3.probe_loop.begin");
    unsigned int probc = 0;
    while (probc < ht_p->slot_capacity && slot_idx != NULL_IDX) {
        hash_slot_t *slot_p = ht_get_slot(ht_p, slot_idx);
        _dbg_print(
            "hash_table_lookup@3.probe_loop.slot%d: "
            "('%.*s', %u, 0x%.8x, %p, %d, %d)",
            slot_idx,
            slot_p->key_len, slot_p->key,
            slot_p->key_len,
            slot_p->hash,
            slot_p->data,
            slot_p->prev_idx,
            slot_p->next_idx
        );
        if (
            ht_p->cntl_arr[slot_idx] != SLOT_OCCUPIED ||
            slot_p->hash != key_p->hash ||
            slot_p->key_len != key_p->key_len
        )
            goto probe_next_slot;
        if (memcmp(slot_p->key, key_p->key, key_p->key_len) == 0) {
            out_slot_handle_p->bucket_idx = bucket_idx;
            out_slot_handle_p->slot_idx = slot_idx;
            _dbg_print(
                "hash_table_lookup@3.probe_loop.ret_success "
                "# (bucket, slot) = (%d, %d)\n",
                bucket_idx, slot_idx
            );
            return slot_idx;
        }
    probe_next_slot:
        _dbg_print("hash_table_lookup@3.probe_loop.probe_next_slot");
        slot_idx = ht_next_slot(ht_p, slot_idx);
        ++probc;
    }

failed_ret:
    _dbg_print("hash_table_lookup@-1.ret # failed\n");
    return NULL_IDX;
}

int hash_table_remove(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
) {
    _dbg_print("hash_table_remove@0._is_valid_storage_ht");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_remove@0.not._is_empty_ht");
    if (_is_empty_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_remove@0._is_valid_ht_slot_handle");
    if (!_is_valid_ht_slot_handle(ht_p, slot_handle_p)) goto failed_ret;

    _dbg_print("hash_table_remove@0._is_slot_occupied");
    if (!_is_slot_occupied(ht_p, slot_handle_p->slot_idx)) goto failed_ret;

    _dbg_print("hash_table_remove@1._ht_assert_intrnl_state");
    _ht_assert_intrnl_state(ht_p);

    _dbg_print("hash_table_remove@2.get_slot # slot, prev, next");
    int *slot_idx_p = &slot_handle_p->slot_idx;
    int *prev_idx_p = &ht_prev_slot(ht_p, *slot_idx_p);
    int *next_idx_p = &ht_next_slot(ht_p, *slot_idx_p);

    _dbg_print("hash_table_remove@2._ht_assert_slot_chain_integrity");
    _ht_assert_slot_chain_integrity(ht_p, *slot_idx_p, *prev_idx_p, *next_idx_p);

    _dbg_print("hash_table_remove@3 # get bucket ptr");
    int *bucket_p = &ht_p->bucket_arr[slot_handle_p->bucket_idx];

    /* unlink */
    _dbg_print("hash_table_remove@4 # unlink");
    if (*prev_idx_p != NULL_IDX)
        ht_next_slot(ht_p, *prev_idx_p) = *next_idx_p;
    else {
        _dbg_print("hash_table_remove@4.b # slot is chain head");
        _dbg_print(
            "bucket_idx, chain_head, slot_idx: %d,%d,%d",
            slot_handle_p->bucket_idx,
            ht_p->bucket_arr[slot_handle_p->bucket_idx],
            slot_handle_p->slot_idx
        );
        assert(ht_p->bucket_arr[slot_handle_p->bucket_idx] == slot_handle_p->slot_idx);
        *bucket_p = *next_idx_p;
    }
    if (*next_idx_p != NULL_IDX)
        ht_prev_slot(ht_p, *next_idx_p) = *prev_idx_p;

    _dbg_print("hash_table_remove@5 # free slot");
    ht_p->slot_free_list[slot_handle_p->slot_idx] = ht_p->free_head_idx;
    ht_p->free_head_idx = slot_handle_p->slot_idx;
    ht_p->cntl_arr[ht_p->free_head_idx] = SLOT_EMPTY;

    _dbg_print("hash_table_remove@6 # destroy handle && --ht_p->slot_count");
    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
    --ht_p->slot_count;

    if (*bucket_p != NULL_IDX) {
        _dbg_print("hash_table_remove@7._ht_assert_slot_chain_integrity");
        int slot_idx = *bucket_p;
        int prev_idx = ht_prev_slot(ht_p, slot_idx);
        int next_idx = ht_next_slot(ht_p, slot_idx);
        _ht_assert_slot_chain_integrity(ht_p, slot_idx, prev_idx, next_idx);
    }

    _dbg_print("hash_table_remove@0.ret_success\n");
    return 0;
failed_ret:
    return -1;
}

const hash_slot_t *hash_table_get_slot(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *slot_handle_p
) {
    _dbg_print("hash_table_get_slot@0._is_valid_storage_ht");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_get_slot@0.not._is_empty_ht");
    if (_is_empty_ht(ht_p)) goto failed_ret;

    _dbg_print("hash_table_get_slot@0._is_valid_ht_slot_handle");
    if (!_is_valid_ht_slot_handle(ht_p, slot_handle_p)) goto failed_ret;

    _dbg_print("hash_table_get_slot@0._is_slot_occupied");
    if (!_is_slot_occupied(ht_p, slot_handle_p->slot_idx)) goto failed_ret;

    _dbg_print("hash_table_get_slot@1._ht_assert_intrnl_state");
    _ht_assert_intrnl_state(ht_p);

    _dbg_print("hash_table_get_slot@2._ht_assert_slot_chain_integrity");
    int slot_idx = slot_handle_p->slot_idx;
    int prev_idx = ht_prev_slot(ht_p, slot_idx);
    int next_idx = ht_next_slot(ht_p, slot_idx);
    _ht_assert_slot_chain_integrity(ht_p, slot_idx, prev_idx, next_idx);

    _dbg_print("hash_table_get_slot@0.ret # ht_get_slot\n");
    return ht_get_slot(ht_p, slot_handle_p->slot_idx);
failed_ret:
    _dbg_print("hash_table_get_slot@-1.ret\n");
    return NULL;
}

unsigned int hash_table_slot_capacity(const hash_table_t *ht_p) {
    return (!ht_p) ? 0 : ht_p->slot_capacity;
}
unsigned int hash_table_slot_count(const hash_table_t *ht_p) {
    return (!ht_p) ? 0 : ht_p->slot_count;
}


/*
 * Implm of now-owning hash table with
 * stable slot idx and double libked list chaining
 */

#include "_libkv_intrnl.h"
#include <kvht.h>

struct _kvht {
    kvht_slot_t    *slot_arr;
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


static inline void
_kvht_dbg_print_slot(int slot_idx, kvht_slot_t *slot_p) {
    _dbg_print(
        "slot[%d]:\t('%.*s', %u, 0x%.8x, %p, %d, %d)",
        slot_idx,
        slot_p->key_len, slot_p->key,
        slot_p->key_len,
        slot_p->hash,
        slot_p->data,
        slot_p->prev_idx,
        slot_p->next_idx
    );
    (void)slot_idx;
    (void)slot_p;
}

kvht_t *
create_kvht(
    unsigned int bucket_count, 
    unsigned int slot_capacity
) {
    if (bucket_count < HT_MIN_NBUCKET) bucket_count = HT_MIN_NBUCKET;
    if (slot_capacity < HT_MIN_NSLOT) slot_capacity = HT_MIN_NSLOT;

    kvht_t *ht_p = (kvht_t*)malloc(sizeof(kvht_t));
    _dbg_log_msg("0");
    assert(ht_p);
    if (!ht_p) goto failed;
    memset(ht_p, 0, sizeof(*ht_p)); /* Very important: zero init */

    _dbg_log_msg("1");
    ht_p->slot_arr          = (kvht_slot_t*)malloc(slot_capacity * sizeof(*ht_p->slot_arr));
    ht_p->slot_free_list    = (int*)malloc(slot_capacity * sizeof(*ht_p->slot_free_list));
    ht_p->cntl_arr          = (unsigned char*)malloc(slot_capacity * sizeof(*ht_p->cntl_arr));
    ht_p->bucket_arr        = (int*)malloc(bucket_count * sizeof(*ht_p->bucket_arr));
    assert(ht_p->slot_arr);
    assert(ht_p->slot_free_list);
    assert(ht_p->cntl_arr);
    assert(ht_p->bucket_arr);
    if (!ht_p->slot_arr || !ht_p->bucket_arr || !ht_p->slot_free_list || !ht_p->cntl_arr)
        goto failed;
    memset(ht_p->bucket_arr, 0xFF, sizeof(*ht_p->bucket_arr) * bucket_count);  // 0xFFFFFFFF == (int32_t)-1

    _dbg_log_msg("2 # loop");
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

    _dbg_log_msg("3");
    ht_p->free_head_idx     = 0;
    ht_p->slot_count        = 0;
    ht_p->slot_capacity     = slot_capacity;
    ht_p->bucket_count      = bucket_count;

    _dbg_log_msg("0.ret\n");
    return ht_p;
failed:
    destroy_kvht(ht_p); /* NULL ptr safe */
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

void destroy_kvht(kvht_t *ht_p) {
    _dbg_log_msg("");
    if (!ht_p) goto scope_end;
    if (ht_p->slot_arr) free(ht_p->slot_arr);
    if (ht_p->cntl_arr) free(ht_p->cntl_arr);
    if (ht_p->slot_free_list) free(ht_p->slot_free_list);
    if (ht_p->bucket_arr) free(ht_p->bucket_arr);
    free(ht_p);
scope_end:
    _dbg_log_msg("ret\n");
    return;
}

#define ht_get_slot(ht_p, slot_idx) \
    &(ht_p)->slot_arr[slot_idx]

#define ht_prev_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].prev_idx

#define ht_next_slot(ht_p, slot_idx) \
    (ht_p)->slot_arr[slot_idx].next_idx


static inline int _is_valid_storage_ht(const kvht_t *ht_p) {
    return (
        ht_p && ht_p->slot_arr && ht_p->cntl_arr &&
        ht_p->slot_free_list && ht_p->bucket_arr
    );
}
static inline void _ht_assert_intrnl_state(const kvht_t *ht_p) {
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
    const kvht_t         *ht_p,
    const kvht_slot_handle_t   *hs_handle_p
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
    const kvht_t *ht_p,
    int                 slot_idx     
) {
    return (
        slot_idx != NULL_IDX &&
        ht_p->cntl_arr[slot_idx] == SLOT_OCCUPIED
    );
}

#define _is_empty_ht(ht_p) (!(ht_p)->slot_count)
#define _is_not_empty_ht(ht_p) (!_is_empty_ht(ht_p))
static inline int _is_full_ht(const kvht_t *ht_p) {
    return (ht_p->slot_count >= ht_p->slot_capacity);
}
#define _ht_has_rem_capacity(ht_p) (!_is_full_ht(ht_p))

static inline void
_ht_assert_slot_chain_integrity(
    const kvht_t   *ht_p,
    int             slot_idx,
    int             prev_idx,
    int             next_idx
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

static inline void
_ht_assert_chain_validity_at(
    const kvht_t   *ht_p,
    int             slot_idx
) {
    if (slot_idx == NULL_IDX) return;
    int prev_idx = ht_prev_slot(ht_p, slot_idx);
    int next_idx = ht_next_slot(ht_p, slot_idx);
    _ht_assert_slot_chain_integrity(ht_p, slot_idx, prev_idx, next_idx);
}


int kvht_insert(
    kvht_t             *ht_p,
    const kvht_key_t   *key_p,
    const void         *data
) {
    _dbg_log_msg("check ht valid storage #0");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("check ht not full");
    if (_is_full_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("validate hash key");
    if (!_is_valid_kvht_key(key_p)) goto failed_ret;
    _dbg_log_msg("assert ht intrnl state #1");
    _ht_assert_intrnl_state(ht_p);

    _dbg_log_msg("#2");
    int bucket_idx          = key_p->hash % ht_p->bucket_count;
    int *head_idx_p         = &ht_p->bucket_arr[bucket_idx];
    int *head_prev_idx_p    = &ht_prev_slot(ht_p, *head_idx_p);

    _dbg_log_msg("free list alloc slot#3");
    int new_slot                = ht_p->free_head_idx;
    ht_p->free_head_idx         = ht_p->slot_free_list[new_slot];
    ht_p->cntl_arr[new_slot]    = SLOT_OCCUPIED;

    _dbg_log_msg("deref slot #4");
    kvht_slot_t *slot_p = ht_get_slot(ht_p, new_slot);
    
    _dbg_log_msg("fill slot #5");
    slot_p->key         = key_p->key_buf;   // non owning
    slot_p->key_len     = key_p->key_len;
    slot_p->hash        = key_p->hash;
    slot_p->data        = data;             // non owning
    slot_p->prev_idx    = NULL_IDX;
    slot_p->next_idx    = *head_idx_p;

    _kvht_dbg_print_slot(new_slot, slot_p);

    _dbg_log_msg("chain push front #6");
    if (*head_idx_p != NULL_IDX)
        *head_prev_idx_p = new_slot;
    *head_idx_p = new_slot;

    _dbg_log_msg("ht assert chain integrity #7");
    _ht_assert_chain_validity_at(ht_p, *head_idx_p);

    _dbg_log_msg("inc slot count #8");
    ++ht_p->slot_count;

    _dbg_log_msg("0.ret\n");
    return new_slot;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL_IDX;
}

int kvht_lookup(
    const kvht_t       *ht_p,
    const kvht_key_t   *key_p,
    kvht_slot_handle_t *out_slot_handle_p
) {
    _dbg_log_msg("validate ht storage #0");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("check ht not empty");
    if (_is_empty_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("validate hash key");
    if (!_is_valid_kvht_key(key_p)) goto failed_ret;
    _dbg_log_msg("check if slot handle null");
    if (!out_slot_handle_p) goto failed_ret;
    _dbg_log_msg("assert intrnl state #1");
    _ht_assert_intrnl_state(ht_p);

    out_slot_handle_p->bucket_idx = NULL_IDX;
    out_slot_handle_p->slot_idx = NULL_IDX;

    int bucket_idx = key_p->hash % ht_p->bucket_count;
    int slot_idx = ht_p->bucket_arr[bucket_idx];
    _dbg_print("bucket_idx=%d; chain_head_idx=%d", bucket_idx, slot_idx);

    _dbg_log_msg("probe_loop.begin #2");
    unsigned int probc = 0;
    while (probc < ht_p->slot_capacity && slot_idx != NULL_IDX) {
        _dbg_log_msg("probe_loop.it");
        kvht_slot_t *slot_p = ht_get_slot(ht_p, slot_idx);
        _kvht_dbg_print_slot(slot_idx, slot_p);
        if (
            ht_p->cntl_arr[slot_idx] != SLOT_OCCUPIED ||
            slot_p->hash != key_p->hash ||
            slot_p->key_len != key_p->key_len
        )
            goto probe_next_slot;
        if (memcmp(slot_p->key, key_p->key_buf, key_p->key_len) == 0) {
            out_slot_handle_p->bucket_idx = bucket_idx;
            out_slot_handle_p->slot_idx = slot_idx;
            _dbg_log_msg("slot found #ret\n");
            _dbg_print("(bucket, slot) = (%d, %d)\n", bucket_idx, slot_idx);
            return slot_idx;
        }
    probe_next_slot:
        _dbg_log_msg("probe_loop.next");
        slot_idx = ht_next_slot(ht_p, slot_idx);
        ++probc;
    }

failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL_IDX;
}

int kvht_remove(
    kvht_t       *ht_p,
    kvht_slot_handle_t *slot_handle_p
) {
    _dbg_log_msg("validate ht storage #0");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("check ht not empty");
    if (_is_empty_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("validate ht slot handle");
    if (!_is_valid_ht_slot_handle(ht_p, slot_handle_p)) goto failed_ret;
    _dbg_log_msg("check if slot occupied");
    if (!_is_slot_occupied(ht_p, slot_handle_p->slot_idx)) goto failed_ret;
    _dbg_log_msg("assert ht intrnl state #1");
    _ht_assert_intrnl_state(ht_p);

    _dbg_log_msg("get slot self, prev, next #2");
    int *slot_idx_p = &slot_handle_p->slot_idx;
    int *prev_idx_p = &ht_prev_slot(ht_p, *slot_idx_p);
    int *next_idx_p = &ht_next_slot(ht_p, *slot_idx_p);

    _dbg_log_msg("assert slot chain integrity #3");
    _ht_assert_slot_chain_integrity(ht_p, *slot_idx_p, *prev_idx_p, *next_idx_p);

    _dbg_log_msg("get bucket ptr");
    int *bucket_p = &ht_p->bucket_arr[slot_handle_p->bucket_idx];
    /* unlink */
    _dbg_log_msg("unlink slot #5");
    if (*prev_idx_p != NULL_IDX)
        ht_next_slot(ht_p, *prev_idx_p) = *next_idx_p;
    else {
        _dbg_log_msg("slot is chain head #5.b");
        _dbg_print(
            "(bucket_idx,chain_head,slot_idx)=(%d,%d,%d)",
            slot_handle_p->bucket_idx,
            ht_p->bucket_arr[slot_handle_p->bucket_idx],
            slot_handle_p->slot_idx
        );
        assert(ht_p->bucket_arr[slot_handle_p->bucket_idx] == slot_handle_p->slot_idx);
        *bucket_p = *next_idx_p;
    }
    if (*next_idx_p != NULL_IDX)
        ht_prev_slot(ht_p, *next_idx_p) = *prev_idx_p;

    _dbg_log_msg("free slot #6");
    ht_p->slot_free_list[slot_handle_p->slot_idx]   = ht_p->free_head_idx;
    ht_p->free_head_idx                             = slot_handle_p->slot_idx;
    ht_p->cntl_arr[ht_p->free_head_idx]             = SLOT_EMPTY;

    _dbg_log_msg("reset handle && dec slot_count #7");
    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
    --ht_p->slot_count;

    _dbg_log_msg("assert ht slot chain integrity #8");
    _ht_assert_chain_validity_at(ht_p, *bucket_p);

    _dbg_log_msg("0.ret\n");
    return 0;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return -1;
}

const kvht_slot_t *kvht_get_slot(
    const kvht_t               *ht_p,
    const kvht_slot_handle_t   *slot_handle_p
) {
    _dbg_log_msg("validate ht storage #0");
    if (!_is_valid_storage_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("check ht not empty");
    if (_is_empty_ht(ht_p)) goto failed_ret;
    _dbg_log_msg("validate slot handle");
    if (!_is_valid_ht_slot_handle(ht_p, slot_handle_p)) goto failed_ret;
    _dbg_log_msg("check slot occupied");
    if (!_is_slot_occupied(ht_p, slot_handle_p->slot_idx)) goto failed_ret;
    _dbg_log_msg("assert ht intrnl state");
    _ht_assert_intrnl_state(ht_p);
    _dbg_log_msg("assert chain integrity #2");
    _ht_assert_chain_validity_at(ht_p, slot_handle_p->slot_idx);

    _dbg_log_msg("0.ret\n");
    return ht_get_slot(ht_p, slot_handle_p->slot_idx);
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

unsigned int kvht_slot_capacity(const kvht_t *ht_p) {
    return (!ht_p) ? 0 : ht_p->slot_capacity;
}
unsigned int kvht_slot_count(const kvht_t *ht_p) {
    return (!ht_p) ? 0 : ht_p->slot_count;
}


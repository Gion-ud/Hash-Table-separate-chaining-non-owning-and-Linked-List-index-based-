#pragma once
#include "hash_table.h"

static const void *_ht_get_data_hash_key(
    const hash_table_t *ht_p,
    const hash_key_t   *hk_p
);
static const void *_ht_get_data_cstr_key(
    const hash_table_t *ht_p,
    const char         *key
);
static int _ht_put_cstr_key(
    hash_table_t   *ht_p,
    const char     *key,
    const void     *data
);
static int _ht_erase_cstr_key(
    hash_table_t   *ht_p,
    const char     *key
);

#ifdef _USING_HASH_TABLE_UTILS

#define ht_get(ht_p, key) _ht_get_data_cstr_key(ht_p, key)
#define ht_put(ht_p, key, data) _ht_put_cstr_key(ht_p, key, data)
#define ht_erase(ht_p, key) _ht_erase_cstr_key(ht_p, key)

static const void *_ht_get_data_hash_key(
    const hash_table_t *ht_p,
    const hash_key_t   *hk_p
) {
    if (!ht_p || !hk_p) return NULL;
    hash_slot_handle_t hs_hndl = {0};
    int ret = hash_table_lookup(ht_p, hk_p, &hs_hndl);
    if (ret < 0) return NULL;
    const hash_slot_t *slot_p = hash_table_get_slot(ht_p, &hs_hndl);
    if (!slot_p) return NULL;
    return slot_p->data;
}

static const void *_ht_get_data_cstr_key(
    const hash_table_t *ht_p,
    const char         *key
) {
    if (!ht_p || !key) return NULL;
    hash_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    return _ht_get_data_hash_key(ht_p, &hk);
}

static int _ht_put_cstr_key(
    hash_table_t   *ht_p,
    const char     *key,
    const void     *data
) {
    if (!ht_p || !key) return -1;
    hash_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    return hash_table_insert(ht_p, &hk, data);
}

static int _ht_erase_cstr_key(
    hash_table_t   *ht_p,
    const char     *key
) {
    if (!ht_p || !key) return -1;
    hash_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    hash_slot_handle_t hs_hndl = {0};
    int ret = hash_table_lookup(ht_p, &hk, &hs_hndl);
    if (ret < 0) return -1;
    return hash_table_remove(ht_p, &hs_hndl);
}

static void ht_slot_handle_reset(
    hash_slot_handle_t *slot_handle_p
) {
    if (!slot_handle_p) return;
    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
}

#endif /* _USING_HASH_TABLE_UTILS */
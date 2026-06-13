#pragma once
#include "kvht.h"

static const void *_kvkvht_get_data_hash_key(
    const kvht_t       *ht_p,
    const kvht_key_t   *hk_p
);
static const void *_kvkvht_get_data_cstr_key(
    const kvht_t *ht_p,
    const char   *key
);
static int _kvht_put_cstr_key(
    kvht_t         *ht_p,
    const char     *key,
    const void     *data
);
static int _kvht_erase_cstr_key(
    kvht_t         *ht_p,
    const char     *key
);

#ifdef _USING_HASH_TABLE_UTILS

#define kvht_get(ht_p, key) _kvkvht_get_data_cstr_key(ht_p, key)
#define kvht_put(ht_p, key, data) _kvht_put_cstr_key(ht_p, key, data)
#define kvht_erase(ht_p, key) _kvht_erase_cstr_key(ht_p, key)

static const void *_kvkvht_get_data_hash_key(
    const kvht_t       *ht_p,
    const kvht_key_t   *hk_p
) {
    if (!ht_p || !hk_p) return NULL;
    kvht_slot_handle_t hs_hndl = {0};
    int ret = hash_table_lookup(ht_p, hk_p, &hs_hndl);
    if (ret < 0) return NULL;
    const kvht_slot_t *slot_p = hash_table_get_slot(ht_p, &hs_hndl);
    if (!slot_p) return NULL;
    return slot_p->data;
}

static const void *_kvkvht_get_data_cstr_key(
    const kvht_t   *ht_p,
    const char     *key
) {
    if (!ht_p || !key) return NULL;
    kvht_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    return _kvkvht_get_data_hash_key(ht_p, &hk);
}

static int _kvht_put_cstr_key(
    kvht_t         *ht_p,
    const char     *key,
    const void     *data
) {
    if (!ht_p || !key) return -1;
    kvht_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    return hash_table_insert(ht_p, &hk, data);
}

static int _kvht_erase_cstr_key(
    kvht_t     *ht_p,
    const char *key
) {
    if (!ht_p || !key) return -1;
    kvht_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key);
    kvht_slot_handle_t hs_hndl = {0};
    int ret = hash_table_lookup(ht_p, &hk, &hs_hndl);
    if (ret < 0) return -1;
    return hash_table_remove(ht_p, &hs_hndl);
}

static void kvht_slot_handle_reset(
    kvht_slot_handle_t *slot_handle_p
) {
    if (!slot_handle_p) return;
    slot_handle_p->bucket_idx = NULL_IDX;
    slot_handle_p->slot_idx = NULL_IDX;
}

#endif /* _USING_HASH_TABLE_UTILS */
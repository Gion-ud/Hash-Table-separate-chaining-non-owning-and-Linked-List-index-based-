#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _hash_slot hash_slot_t;
typedef struct _hash_table hash_table_t;

#define HT_MIN_NSLOT 4
#define HT_MIN_NBUCKET 4


struct _hash_slot {
    const char     *key;        // not owned; DO NOT modify
    unsigned int    key_len;
    uint32_t        hash;
    const void     *data;       // not owned
    int             prev_idx;
    int             next_idx;
};

typedef struct _hash_key {
    const char     *key;        // not owned
    unsigned int    key_len;
    uint32_t        hash;
} hash_key_t;

#define NULL_IDX -1

extern hash_table_t *create_hash_table(
    unsigned int bucket_capacity, 
    unsigned int slot_capacity
);
extern void destroy_hash_table(hash_table_t *ht_p);

typedef struct _hash_slot_handle_t {
    int     slot_idx;
    int     bucket_idx;
} hash_slot_handle_t;

extern int hash_table_insert(
    hash_table_t   *ht_p,
    hash_key_t     *key_p,
    const void     *data
);
extern int hash_table_lookup(
    const hash_table_t     *ht_p,
    hash_key_t             *key_p,
    hash_slot_handle_t     *out_slot_handle_p
);
extern int hash_table_destroy_handle(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
);
extern int hash_table_remove(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
);
extern const hash_slot_t *hash_table_get_slot(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *slot_handle_p
);

#include <string.h>
#include <hash_fn.h>
#define hash32 fnv_1a_hash32

static const hash_key_t *
make_hash_key_from_cstr(
    hash_key_t *in_hash_key_p,
    const char *cstr
) {
    if (!in_hash_key_p || !cstr) return NULL;
    in_hash_key_p->key = cstr;
    in_hash_key_p->key_len = strlen(cstr);
    in_hash_key_p->hash = hash32(in_hash_key_p->key, in_hash_key_p->key_len);
    return in_hash_key_p;
}

static const void *hash_table_get_data_from_key(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *key_p
) {
    if (!ht_p || !key_p) return NULL;
    hash_slot_handle_t hs_hndl = {0};
    int ret = hash_table_lookup(ht_p, key_p, &hs_hndl);
    if (ret < 0) return NULL;
    const hash_slot_t *slot_p = hash_table_get_slot(ht_p, &hs_hndl);
    if (!slot_p) return NULL;
    hash_table_destroy_handle(ht_p, &hs_hndl);
    return slot_p->data;
}

static const void *ht_get_data_from_cstr_key(
    const hash_table_t *ht_p,
    const char         *key_cstr
) {
    if (!ht_p || !key_cstr) return NULL;
    hash_slot_handle_t hs_hndl = {0};
    hash_key_t hk = {0};
    make_hash_key_from_cstr(&hk, key_cstr);
    int ret = hash_table_lookup(ht_p, &hk, &hs_hndl);
    if (ret < 0) return NULL;
    const hash_slot_t *slot_p = hash_table_get_slot(ht_p, &hs_hndl);
    if (!slot_p) return NULL;
    hash_table_destroy_handle(ht_p, &hs_hndl);
    return slot_p->data;
}

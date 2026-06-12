#pragma once

#include <stddef.h>
#include <stdint.h>
#include "hash_key.h"

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
    hash_table_t       *ht_p,
    const hash_key_t   *key_p,
    const void         *data
);
extern int hash_table_lookup(
    const hash_table_t     *ht_p,
    const hash_key_t       *key_p,
    hash_slot_handle_t     *out_slot_handle_p
);

extern int hash_table_remove(
    hash_table_t       *ht_p,
    hash_slot_handle_t *slot_handle_p
);
extern const hash_slot_t *hash_table_get_slot(
    const hash_table_t         *ht_p,
    const hash_slot_handle_t   *slot_handle_p
);


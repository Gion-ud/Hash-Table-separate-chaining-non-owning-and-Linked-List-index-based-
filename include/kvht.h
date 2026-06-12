#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kvht_key.h"

typedef struct _kvht_slot kvht_slot_t;
typedef struct _kvht kvht_t;

#define HT_MIN_NSLOT 4
#define HT_MIN_NBUCKET 4


struct _kvht_slot {
    const char     *key;        // not owned; DO NOT modify
    unsigned int    key_len;
    uint32_t        hash;
    const void     *data;       // not owned
    int             prev_idx;
    int             next_idx;
};

#define NULL_IDX -1

extern kvht_t *create_kvht(
    unsigned int bucket_capacity, 
    unsigned int slot_capacity
);
extern void destroy_kvht(kvht_t *ht_p);

typedef struct _kvht_slot_handle_t {
    int     slot_idx;
    int     bucket_idx;
} kvht_slot_handle_t;

extern int kvht_insert(
    kvht_t             *ht_p,
    const kvht_key_t   *key_p,
    const void         *data
);
extern int kvht_lookup(
    const kvht_t           *ht_p,
    const kvht_key_t       *key_p,
    kvht_slot_handle_t     *out_slot_handle_p
);

extern int kvht_remove(
    kvht_t             *ht_p,
    kvht_slot_handle_t *slot_handle_p
);
extern const kvht_slot_t *kvht_get_slot(
    const kvht_t               *ht_p,
    const kvht_slot_handle_t   *slot_handle_p
);


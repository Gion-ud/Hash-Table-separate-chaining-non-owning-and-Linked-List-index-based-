#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _kvht_key {
    const char     *key;        // not owned
    unsigned int    key_len;
    uint32_t        hash;
} kvht_key_t;

#if !defined(_HASH_TABLE_INTRNL_IMPLM)

#include <string.h>
#include <hash_fn.h>
#define hash32 fnv_1a_hash32

static const kvht_key_t *
make_hash_key_from_cstr(
    kvht_key_t *in_hash_key_p,
    const char *cstr
) {
    if (!in_hash_key_p || !cstr) return NULL;
    in_hash_key_p->key = cstr;
    in_hash_key_p->key_len = strlen(cstr);
    in_hash_key_p->hash = hash32(in_hash_key_p->key, in_hash_key_p->key_len);
    return in_hash_key_p;
}

#endif /*_HASH_TABLE_INTRNL_IMPLM*/

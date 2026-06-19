#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _kvht_key {
    const char     *key_buf;        // not owned
    unsigned int    key_len;
    uint32_t        hash;
} kvht_key_t;

#define _USING_32BIT_HASH

#if !defined(_KVHT_INTRNL_IMPLM)

#include <string.h>
#include <hash_fn.h>
#define hash32 fnv_1a_hash32

static const kvht_key_t *
make_hash_key_from_cstr(
    kvht_key_t *in_hash_key_p,
    const char *cstr
) {
    if (!in_hash_key_p || !cstr) return NULL;
    in_hash_key_p->key_buf  = cstr;
    in_hash_key_p->key_len  = strlen(cstr);
    in_hash_key_p->hash     = hash32(in_hash_key_p->key_buf, in_hash_key_p->key_len);
    return in_hash_key_p;
}
#else
#define _is_valid_kvht_key(hk_p) ((hk_p) && (hk_p)->key_buf && (hk_p)->key_len)
#endif /*_KVHT_INTRNL_IMPLM*/

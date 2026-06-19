#include "_libkv_intrnl.h"
#include <kvht_key.h>
#include <kvtbl.h>
#include <kvht.h>
#include <kvimg.h>
#include <kvfile.h>
#include <kvarena.h>
#include <stdint.h>

#include "_kvarena.inl"
#include "dbg_print.h"

struct KVTable {
    kvht_t     *ht_p;
    KVArena    *kva_p;
    uint32_t    size;
    uint32_t    delcnt;
    uint32_t    capacity;
};

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define _KVTA_DEFAULT_BUFSIZE 1024u
#define _KVTA_ALIGN 4u



KVTable *Create_KVTable(
    uint32_t    entry_capacity
) {
    _dbg_log_msg("");
    if (!entry_capacity) entry_capacity = 1;
    __auto_type kvtbl_p = (KVTable*)malloc(sizeof(KVTable));
    if (!kvtbl_p) goto failed_ret;
    memset(kvtbl_p, 0, sizeof(*kvtbl_p));

    kvtbl_p->ht_p = create_kvht(entry_capacity, entry_capacity);
    kvtbl_p->kva_p = create_kvarena(
        _KVTA_DEFAULT_BUFSIZE, entry_capacity, _KVTA_ALIGN
    );
    if (!kvtbl_p->ht_p || !kvtbl_p->kva_p) goto failed;
    kvtbl_p->size       = 0;
    kvtbl_p->delcnt     = 0;
    kvtbl_p->capacity   = entry_capacity;

    _dbg_log_msg("0.ret");
    return kvtbl_p;
failed:
    Destroy_KVTable(kvtbl_p);
failed_ret:
    _dbg_log_msg("-1.ret");
    return NULL;
}

void Destroy_KVTable(
    KVTable *kvtbl_p
) {
    _dbg_log_msg("");
    if (kvtbl_p) return;
    if (kvtbl_p->ht_p) free(kvtbl_p->ht_p);
    if (kvtbl_p->kva_p) free(kvtbl_p->kva_p);
    free(kvtbl_p);
}

LIBKV_INTRNL kvht_t *_KVTable_DupHashTable(
    KVTable    *kvtbl_p,
    uint32_t    new_ht_capacity
) {
    _dbg_log_msg("");
    kvht_t *new_ht_p = create_kvht(new_ht_capacity, new_ht_capacity);
    if (!new_ht_p) goto failed_ret;

    for (uint32_t i = 0u; i < kvtbl_p->capacity; ++i) {
        KVArenaEntryView kva_ev = {0};
        const KVArenaEntry *kva_ent_p = kvarena_get_entry(kvtbl_p->kva_p, i);
        if (!kva_ent_p) {
            assert(kvtbl_p->kva_p->cntl_arr[i] == KVA_ENT_DEAD);
            continue;
        }
        kvarena_entry_to_entview(kvtbl_p->kva_p, kva_ent_p, &kva_ev);
        kvht_key_t key = {
            .key_buf    = kva_ev.key_p,
            .key_len    = kva_ev.key_len,
            .hash       = kva_ev.key_hash
        };
        int rc = kvht_insert(new_ht_p, &key, kva_ent_p);
        if (rc < 0) goto failed;
    }

    return new_ht_p;
failed:
    destroy_kvht(new_ht_p);
failed_ret:
    return NULL;
}

int KVTable_Insert(
    KVTable            *kvtbl_p,
    const kvht_key_t   *kvht_key_p,
    const void         *data,
    uint32_t            data_len
) {
    _dbg_log_msg("");
    if (!kvtbl_p) goto failed_ret;
    assert(kvtbl_p->ht_p && kvtbl_p->kva_p);
    assert(kvtbl_p->size <= kvtbl_p->capacity);
    assert(kvtbl_p->delcnt <= kvtbl_p->capacity);
    assert(kvtbl_p->capacity <= kvarena_capacity(kvtbl_p->kva_p));

    kvht_slot_handle_t hsh = {0};
    if (kvht_lookup(kvtbl_p->ht_p, kvht_key_p, &hsh) >= 0)
        return 0;

    int32_t kva_ent_idx = 
        kvarena_push_auto_grow(kvtbl_p->kva_p, kvht_key_p, data, data_len);

    if (kva_ent_idx == NULL_IDX) goto failed_ret;
    
    if (kvtbl_p->size == kvtbl_p->capacity) {
        uint32_t new_cap = kvtbl_p->capacity * 2;
        kvht_t *new_ht_p = _KVTable_DupHashTable(kvtbl_p, new_cap);
        if (!new_ht_p) {
            kvarena_mark_dead(kvtbl_p->kva_p, kva_ent_idx);
            goto failed_ret;
        }
        destroy_kvht(kvtbl_p->ht_p);
        kvtbl_p->ht_p = new_ht_p;
        kvtbl_p->capacity = new_cap;
    }

    __auto_type kva_ent_p = kvarena_get_entry(kvtbl_p->kva_p, kva_ent_idx);
    assert(kva_ent_p);

    int rc = kvht_insert(kvtbl_p->ht_p, kvht_key_p, kva_ent_p);
    if (rc < 0) {
        kvarena_mark_dead(kvtbl_p->kva_p, kva_ent_idx);
        goto failed_ret;
    }

    ++kvtbl_p->size;
    return 0;
failed_ret:
    return -1;
}

int KVTable_Compact(
    KVTable    *kvtbl_p
) {
    _dbg_log_msg("");
    if (!kvtbl_p) goto failed_ret;
    assert(kvtbl_p->ht_p && kvtbl_p->kva_p);
    assert(kvtbl_p->delcnt <= kvtbl_p->capacity);
    assert(kvtbl_p->size <= kvtbl_p->capacity);
    assert(kvtbl_p->capacity <= kvarena_capacity(kvtbl_p->kva_p));

    int rc = kvarena_compact(&kvtbl_p->kva_p);
    if (rc < 0) goto failed_ret;
    destroy_kvht(kvtbl_p->ht_p);
    kvtbl_p->ht_p = create_kvht(kvtbl_p->capacity, kvtbl_p->capacity);
    if (!kvtbl_p->ht_p) goto failed_ret;

    uint32_t kva_size = kvarena_size(kvtbl_p->kva_p);
    for (uint32_t i = 0u; i < kva_size; ++i) {
        KVArenaEntryView kva_ev = {0};
        __auto_type __kva_ent_p = kvarena_get_entry(kvtbl_p->kva_p, i);
        assert(__kva_ent_p);
        kvarena_entry_to_entview(kvtbl_p->kva_p, __kva_ent_p, &kva_ev);
        kvht_key_t hk = {
            .key_buf    = kva_ev.key_p,
            .key_len    = kva_ev.key_len,
            .hash       = kva_ev.key_hash,
        };
        rc = kvht_insert(kvtbl_p->ht_p, &hk, __kva_ent_p);
        if (rc < 0) goto failed;
    }

    kvtbl_p->delcnt = 0;
    kvtbl_p->size   = kva_size;

    return 0;
failed:
    destroy_kvht(kvtbl_p->ht_p);
failed_ret:
    return -1;
}

int KVTable_Remove(
    KVTable            *kvtbl_p,
    const kvht_key_t   *kvht_key_p
) {
    _dbg_log_msg("");
    if (!kvtbl_p || !_is_valid_kvht_key(kvht_key_p))
        goto failed_ret;

    kvht_slot_handle_t slot_hndl = {
        .slot_idx   = NULL_IDX,
        .bucket_idx = NULL_IDX,
    };
    int rc = kvht_lookup(kvtbl_p->ht_p, kvht_key_p, &slot_hndl);
    if (rc < 0) goto failed_ret;
    __auto_type slot_p = kvht_get_slot((const kvht_t*)kvtbl_p->ht_p, &slot_hndl);
    assert(slot_p);
    int32_t kva_ent_idx = (int32_t)(
        (KVArenaEntry*)slot_p->data -
        (KVArenaEntry*)kvarena_entrytbl(kvtbl_p->kva_p)
    );
    rc = kvarena_mark_dead(kvtbl_p->kva_p, kva_ent_idx);
    if (rc < 0) goto failed_ret;
    rc = kvht_remove(kvtbl_p->ht_p, &slot_hndl);
    if (rc < 0) goto failed_ret;

    ++kvtbl_p->delcnt;
    return 0;
failed_ret:
    return -1;
}

const KVTableEntry *KVTable_GetEntry(
    const KVTable      *kvtbl_p,
    const kvht_key_t   *kvht_key_p
) {
    _dbg_log_msg("");
    if (!kvtbl_p || !_is_valid_kvht_key(kvht_key_p))
        goto failed_ret;

    kvht_slot_handle_t slot_hndl = {
        .slot_idx   = NULL_IDX,
        .bucket_idx = NULL_IDX,
    };
    int rc = kvht_lookup(kvtbl_p->ht_p, kvht_key_p, &slot_hndl);
    if (rc < 0) goto failed_ret;
    __auto_type slot_p = kvht_get_slot((const kvht_t*)kvtbl_p->ht_p, &slot_hndl);
    assert(slot_p);

    return (const KVTableEntry*)slot_p->data;
failed_ret:
    return NULL;
}

const KVTableEntryView *KVTable_GetEntryView(
    const KVTable      *kvtbl_p,
    const kvht_key_t   *kvht_key_p,
    KVTableEntryView   *out_ev_p
) {
    _dbg_log_msg("");
    __auto_type kva_ent_p = KVTable_GetEntry(kvtbl_p, kvht_key_p);
    if (!kva_ent_p || !out_ev_p) goto failed_ret;
    return kvarena_entry_to_entview(kvtbl_p->kva_p, kva_ent_p, out_ev_p);
failed_ret:
    return NULL;
}


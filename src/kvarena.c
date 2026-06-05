/* more testing */

#define _HASH_TABLE_INTRNL_IMPLM
#include <kvfile.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include "dbg_print.h"
#include "alignoff.h"
#include "hash_key.h"
#include "kvfile.h"
#include <kvarena.h>

struct KVArena {
    KVFileEntry    *entrytbl;       // [0]
    unsigned char  *data_buf;       // [1]
    uint8_t        *cntl_arr;       // [2]
    uint32_t        align;          // [3]
    uint32_t        entrycnt;       // [4]
    uint32_t        entrycap;       // [5]
    uint32_t        data_buf_len;   // [6]
    uint32_t        data_buf_size;  // [7]
};

uint32_t kvarena_size(KVArena *kva_p) {
    return (!kva_p) ? 0 : kva_p->entrycnt;
}
uint32_t kvarena_capacity(KVArena *kva_p) {
    return (!kva_p) ? 0 : kva_p->entrycap;
}
uint32_t kvarena_data_len(KVArena *kva_p) {
    return (!kva_p) ? 0 : kva_p->data_buf_len;
}
uint32_t kvarena_data_size(KVArena *kva_p) {
    return (!kva_p) ? 0 : kva_p->data_buf_size;
}
int kvarena_is_full(KVArena *kva_p) {
    return (!kva_p) ? 0 : (kva_p->entrycnt >= kva_p->entrycap);
}
int kvarena_is_empty(KVArena *kva_p) {
    return (!kva_p) ? 0 : (kva_p->entrycnt);
}


KVArena *create_kvarena(
    uint32_t    bufsize,
    uint32_t    entrycap,
    uint32_t    align
) {
    _dbg_print("create_kvarena@0");
    if (bufsize < KVA_MIN_BUFSIZE) bufsize = KVA_MIN_BUFSIZE;
    if (entrycap < KVA_MIN_ENTC) entrycap = KVA_MIN_ENTC;
    if (!is_pow2(align)) align = KVA_ALIGN_DEFAULT;

    _dbg_print("create_kvarena@1.malloc.KVArena");
    KVArena *kva_p = (KVArena*)malloc(sizeof(KVArena));
    if (!kva_p) goto failed_ret;
    memset(kva_p, 0, sizeof(*kva_p));

    _dbg_print("create_kvarena@2.alloc_res # entrytbl && data");    
    kva_p->entrytbl = (KVFileEntry*)malloc(entrycap * sizeof(KVFileEntry));
    kva_p->data_buf = (unsigned char*)malloc(bufsize);
    kva_p->cntl_arr = (uint8_t*)malloc(entrycap * sizeof(uint8_t));
    if (!kva_p->entrytbl || !kva_p->data_buf || !kva_p->cntl_arr) goto failed;

    _dbg_print("create_kvarena@3.memset_res");
    memset(kva_p->entrytbl, 0, entrycap * sizeof(*kva_p->entrytbl));
    memset(kva_p->data_buf, 0, bufsize);
    memset(kva_p->cntl_arr, 0, entrycap * sizeof(*kva_p->cntl_arr));

    _dbg_print("create_kvarena@4.set_intrnl_state");
    kva_p->align            = align;
    kva_p->entrycnt         = 0;
    kva_p->entrycap         = entrycap;
    kva_p->data_buf_len     = 0;
    kva_p->data_buf_size    = bufsize;

    _dbg_print("create_kvarena@0.ret\n");
    return kva_p;
failed:
    _dbg_print("create_kvarena@-1.failed.destroy_kvarena");
    destroy_kvarena(kva_p);
failed_ret:
    _dbg_print("create_kvarena@-1.failed_ret\n");
    return NULL;
}

void destroy_kvarena(KVArena *kva_p) {
    _dbg_print("destroy_kvarena@0");
    if (!kva_p) goto dtor_ret;
    _dbg_print("destroy_kvarena@1");
    if (kva_p->entrytbl) free(kva_p->entrytbl);
    _dbg_print("destroy_kvarena@2");
    if (kva_p->data_buf) free(kva_p->data_buf);
    if (kva_p->cntl_arr) free(kva_p->cntl_arr);
dtor_ret:
    _dbg_print("destroy_kvarena@0.ret\n");
    return;
}

static inline void _kva_assert_intrnl_state(KVArena *kva_p) {
    assert(kva_p->entrycap >= KVA_MIN_ENTC);
    assert(kva_p->data_buf_size >= KVA_MIN_BUFSIZE);
    assert(kva_p->entrycnt <= kva_p->entrycap);
    assert(kva_p->data_buf_len <= kva_p->data_buf_size);
    assert(is_pow2(kva_p->align));
    assert(kva_p->data_buf_len % kva_p->align == 0);
}

static int32_t _kvarena_insert_from_entview(
    KVArena            *dest_kva_p,
    KVArenaEntryView   *ent_view_p
) {
    uint32_t entryoff = dest_kva_p->data_buf_len;
    uint32_t key_size_aligned = align_off(ent_view_p->key_len + 1, dest_kva_p->align);
    uint32_t val_size_aligned = align_off(ent_view_p->val_len, dest_kva_p->align);
    uint32_t alloc_size = key_size_aligned + val_size_aligned;
    uint32_t idx = dest_kva_p->entrycnt;

    dest_kva_p->entrytbl[idx].key_hash   = ent_view_p->key_hash;
    dest_kva_p->entrytbl[idx].key_len    = ent_view_p->key_len;
    dest_kva_p->entrytbl[idx].key_off    = entryoff;
    dest_kva_p->entrytbl[idx].val_len    = ent_view_p->val_len;
    dest_kva_p->entrytbl[idx].val_off    = entryoff + key_size_aligned;

    dest_kva_p->cntl_arr[idx]    = KVA_ENT_INUSE;

    unsigned char *_dest_key_p = dest_kva_p->data_buf + dest_kva_p->entrytbl[idx].key_off;
    memcpy(_dest_key_p, ent_view_p->key_p, ent_view_p->key_len);
    _dest_key_p[dest_kva_p->entrytbl[idx].key_len] = '\0'; // NUL termination
    unsigned char *_dest_val_p = dest_kva_p->data_buf + dest_kva_p->entrytbl[idx].val_off;
    memcpy(_dest_val_p, ent_view_p->val_p, dest_kva_p->entrytbl[idx].val_len);

    dest_kva_p->data_buf_len += alloc_size;
    ++dest_kva_p->entrycnt;

    return (int32_t)idx;
}

int32_t kvarena_push(
    KVArena            *kva_p,
    const hash_key_t   *hash_key_p,
    const void         *data,
    uint32_t            data_len
) {
    _dbg_print("kvarena_push@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf)
        goto failed_ret;
    if (!hash_key_p || !hash_key_p->key || !hash_key_p->key_len)
        goto failed_ret;
    if (!data || !data_len) goto failed_ret;

    _dbg_print("kvarena_push@1.check_if_full");
    if (kva_p->entrycnt >= kva_p->entrycap) goto failed_ret;

    _dbg_print("kvarena_push@2.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    _dbg_print("kvarena_push@3");
    uint32_t entryoff = kva_p->data_buf_len;
    uint32_t key_size_aligned = align_off(hash_key_p->key_len + 1, kva_p->align);
    uint32_t data_size_aligned = align_off(data_len, kva_p->align);
    uint32_t alloc_size = key_size_aligned + data_size_aligned;
    uint32_t new_off = entryoff + alloc_size;

    _dbg_print("kvarena_push@4.check_if_realloc_databuf_required");
    if (new_off > kva_p->data_buf_size) {
        uint32_t new_data_buf_size = (alloc_size > kva_p->entrycnt)
            ? new_off * 2 : kva_p->data_buf_len * 2;

        _dbg_print("kvarena_push@4.a.realloc");
        unsigned char *new_data_buf =
            (unsigned char*)realloc(kva_p->data_buf, new_data_buf_size);
        _dbg_print("kvarena_push@4.a.check_realloc_stat");
        if (!new_data_buf) goto failed_ret;

        _dbg_print("kvarena_push@4.a.assign_databuf_with_new_buf");
        kva_p->data_buf = new_data_buf;
        kva_p->data_buf_size = new_data_buf_size;

        _dbg_print("kvarena_push@4.a.success");
    }

    _dbg_print("kvarena_push@5.fill_entry_fields");
    uint32_t idx = kva_p->entrycnt;
    assert(kva_p->cntl_arr[idx] == KVA_ENT_EMPTY);

    kva_p->entrytbl[idx].key_hash   = hash_key_p->hash;
    kva_p->entrytbl[idx].key_len    = hash_key_p->key_len;
    kva_p->entrytbl[idx].key_off    = entryoff;
    kva_p->entrytbl[idx].val_len    = data_len;
    kva_p->entrytbl[idx].val_off    = entryoff + key_size_aligned;

    kva_p->cntl_arr[idx]    = KVA_ENT_INUSE;

    _dbg_print("kvarena_push@6.copy_key");
    unsigned char *_dest_key_p = kva_p->data_buf + kva_p->entrytbl[idx].key_off;
    memcpy(
        _dest_key_p,
        hash_key_p->key,
        kva_p->entrytbl[idx].key_len
    );
    _dest_key_p[kva_p->entrytbl[idx].key_len] = '\0'; // NUL termination

    _dbg_print("kvarena_push@7.copy_data");
    unsigned char *_dest_data_p = kva_p->data_buf + kva_p->entrytbl[idx].val_off;
    memcpy(
        _dest_data_p,
        data,
        kva_p->entrytbl[idx].val_len
    );

    kva_p->data_buf_len += alloc_size;
    ++kva_p->entrycnt;

    _dbg_print("kvarena_push@0.ret\n");
    return (int32_t)idx;
failed_ret:
    _dbg_print("kvarena_push@-1.failed_ret\n");
    return NULL_IDX;
}

int32_t kvarena_push_auto_grow(
    KVArena            *kva_p,
    const hash_key_t   *hash_key_p,
    const void         *data,
    uint32_t            data_len
) {
    _dbg_print("kvarena_push_auto_grow@0.validation");
    if (!kva_p) goto failed_ret;

    if (kva_p->entrycnt >= kva_p->entrycap) {
        _dbg_print("kvarena_push_auto_grow@1a.grow");
        int ret = kvarena_grow(kva_p);
        if (ret < 0) goto failed_ret;
    }
    _dbg_print("kvarena_push_auto_grow@0.ret.kvarena_push\n");
    return kvarena_push(kva_p, hash_key_p, data, data_len);
failed_ret:
    _dbg_print("kvarena_push_auto_grow@-1.failed_ret\n");
    return NULL_IDX;
}

int kvarena_mark_dead(
    KVArena    *kva_p,
    uint32_t    ent_idx
) {
    _dbg_print("kvarena_mark_dead@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf)
        goto failed_ret;
    if (ent_idx >= kva_p->entrycnt)
        goto failed_ret;

    _dbg_print("kvarena_mark_dead@2.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    _dbg_print("kvarena_mark_dead@3.mark_dead");
    kva_p->cntl_arr[ent_idx] = KVA_ENT_DEAD;

    _dbg_print("kvarena_mark_dead@0.ret\n");
    return (int32_t)ent_idx;
failed_ret:
    _dbg_print("kvarena_mark_dead@-1.failed_ret\n");
    return -1;
}

int32_t kvarena_get(
    KVArena            *kva_p,
    uint32_t            ent_idx,
    KVArenaEntryView   *out_entview_p
) {
    _dbg_print("kvarena_get@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf) goto failed_ret;
    if (ent_idx >= kva_p->entrycnt) goto failed_ret;
    if (!out_entview_p) goto failed_ret;
    if (kva_p->cntl_arr[ent_idx] != KVA_ENT_INUSE) goto failed_ret;

    _dbg_print("kvarena_get@2.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    _dbg_print("kvarena_get@3.get_ent");
    KVFileEntry *ent_p = &kva_p->entrytbl[ent_idx];
    out_entview_p->key_hash = ent_p->key_hash;
    out_entview_p->key_p    = (char*)kva_p->data_buf + ent_p->key_off;
    out_entview_p->key_len  = ent_p->key_len;
    out_entview_p->val_p    = kva_p->data_buf + ent_p->val_off;
    out_entview_p->val_len  = ent_p->val_len;

    _dbg_print("kvarena_get@0.ret\n");
    return (int32_t)ent_idx;
failed_ret:
    _dbg_print("kvarena_get@-1.failed_ret\n");
    return NULL_IDX;
}


int kvarena_pop(KVArena *kva_p) {
    _dbg_print("kvarena_pop@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf)
        goto failed_ret;

    _dbg_print("kvarena_pop@1.check_if_empty");
    if (!kva_p->entrycnt) goto failed_ret;

    _dbg_print("kvarena_pop@2.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    _dbg_print("kvarena_pop@3");
    uint32_t old_back_idx = kva_p->entrycnt - 1;
    uint32_t old_back_off = kva_p->entrytbl[old_back_idx].key_off;
    uint32_t _entrylen =
        align_off(kva_p->entrytbl[old_back_idx].key_len + 1, kva_p->align) +
        align_off(kva_p->entrytbl[old_back_idx].val_len, kva_p->align);
    uint32_t _old_back_off = kva_p->data_buf_len - _entrylen;

    assert(kva_p->cntl_arr[old_back_idx] != KVA_ENT_EMPTY);
    assert(old_back_off % kva_p->align == 0);
    _dbg_print("kvarena_pop@3 # old_back_off: %u; _old_back_off: %u", old_back_off, _old_back_off);
    assert(old_back_off == _old_back_off);

    _dbg_print("kvarena_pop@4");
    unsigned char *old_back_blob_p = kva_p->data_buf + old_back_off;
    memset(old_back_blob_p, 0, _entrylen);

    _dbg_print("kvarena_pop@5");
    kva_p->cntl_arr[old_back_idx] = 0u;
    kva_p->data_buf_len -= _entrylen;
    --kva_p->entrycnt;

    _dbg_print("kvarena_pop@0.ret\n");
    return 0;
failed_ret:
    _dbg_print("kvarena_pop@-1.failed_ret\n");
    return -1;
}

int kvarena_grow(KVArena *kva_p) {
    _dbg_print("kvarena_grow@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf)
        goto failed_ret;

    _dbg_print("kvarena_grow@1.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    uint32_t new_entrycap = kva_p->entrycap * 2;
    KVFileEntry *old_entrytbl = kva_p->entrytbl;
    uint8_t *old_cntl_arr = kva_p->cntl_arr;

    _dbg_print("kvarena_grow@2.malloc.new_entrytbl");
    KVFileEntry *new_entrytbl = (KVFileEntry*)malloc(new_entrycap * sizeof(KVFileEntry));
    if (!new_entrytbl) goto failed_ret;

    _dbg_print("kvarena_grow@2.malloc.new_cntl_arr");
    uint8_t *new_cntl_arr = (uint8_t*)malloc(new_entrycap * sizeof(uint8_t));
    if (!new_cntl_arr) {
        free(new_entrytbl);
        goto failed_ret;
    }

    _dbg_print("kvarena_grow@3.memset");
    memset(new_entrytbl, 0, new_entrycap * sizeof(*new_entrytbl));
    memset(new_cntl_arr, 0, new_entrycap * sizeof(*new_cntl_arr));
    _dbg_print("kvarena_grow@4.memcpy");
    memcpy(new_entrytbl, old_entrytbl, kva_p->entrycnt * sizeof(*new_entrytbl));
    memcpy(new_cntl_arr, old_cntl_arr, kva_p->entrycnt * sizeof(*new_cntl_arr));
    _dbg_print("kvarena_grow@5.free");
    free(old_entrytbl);
    free(old_cntl_arr);
    _dbg_print("kvarena_grow@6.reassign");
    kva_p->entrytbl = new_entrytbl;
    kva_p->cntl_arr = new_cntl_arr;
    kva_p->entrycap = new_entrycap;

    _dbg_print("kvarena_grow@0.ret\n");
    return 0;
failed_ret:
    _dbg_print("kvarena_grow@-1.failed_ret\n");
    return -1;
}

int kvarena_compact(KVArena **kva_pp) {
    _dbg_print("kvarena_compact@0.validation");
    if (!kva_pp || !*kva_pp || !(*kva_pp)->entrytbl || !(*kva_pp)->data_buf)
        goto failed_ret;

    KVArena *old_kva_p = *kva_pp;

    _dbg_print("kvarena_compact@1.assert_intrnl_state");
    _kva_assert_intrnl_state(old_kva_p);

    _dbg_print("kvarena_compact@2.create_kvarena");
    KVArena *new_kva_p =
        create_kvarena(
            old_kva_p->data_buf_size,
            old_kva_p->entrycap,
            old_kva_p->align
        );
    if (!new_kva_p) goto failed_ret;


    _dbg_print("kvarena_compact@3.loop");
    for (uint32_t i = 0; i < old_kva_p->entrycnt; ++i) {
        if (old_kva_p->cntl_arr[i] == KVA_ENT_DEAD) continue;
        KVFileEntry *ent_p = &old_kva_p->entrytbl[i];
        KVArenaEntryView ent_view = {
            .key_hash   = ent_p->key_hash,
            .key_p      = (char*)old_kva_p->data_buf + ent_p->key_off,
            .key_len    = ent_p->key_len,
            .val_p      = old_kva_p->data_buf + ent_p->val_off,
            .val_len    = ent_p->val_len
        };
        _kvarena_insert_from_entview(new_kva_p, &ent_view);
    }

    _dbg_print("kvarena_compact@4.destroy_kvarena");
    destroy_kvarena(old_kva_p);
    *kva_pp = new_kva_p;

    _dbg_print("kvarena_compact@0.ret\n");
    return 0;
failed_ret:
    _dbg_print("kvarena_compact@-1.failed_ret\n");
    return -1;
}


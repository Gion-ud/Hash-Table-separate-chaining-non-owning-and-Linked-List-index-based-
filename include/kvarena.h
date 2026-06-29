#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kvht_key.h"
#include "kvfile.h"
#include "_kvapi.h"

typedef KVFileEntry KVArenaEntry;


#ifndef NULL_IDX
#define NULL_IDX -1
#endif

#define KVA_MIN_ENTC        4
#define KVA_MIN_BUFSIZE     4
#define KVA_ALIGN_DEFAULT   4

#define KVA_ENT_EMPTY   0
#define KVA_ENT_INUSE   1
#define KVA_ENT_DEAD    2

typedef struct KVArena KVArena;
typedef KVArenaEntry *KVArenaIterator;

typedef struct KVArenaEntryView {
    uint32_t    key_hash;
    const char *key_p;
    uint32_t    key_len;
    const void *val_p;
    uint32_t    val_len;
} KVArenaEntryView;

LIBKV_API KVArena *create_kvarena(
    uint32_t    bufsize,
    uint32_t    entrycap,
    uint32_t    align
);
LIBKV_API void destroy_kvarena(KVArena *kva_p);
LIBKV_API int32_t kvarena_push(
    KVArena            *kva_p,
    const kvht_key_t   *kvht_key_p,
    const void         *data,
    uint32_t            data_len
);
LIBKV_API int kvarena_pop(KVArena *kva_p);
LIBKV_API int kvarena_grow(KVArena *kva_p);
LIBKV_API int kvarena_mark_dead(
    KVArena    *kva_p,
    uint32_t    ent_idx
);
LIBKV_API int kvarena_compact(KVArena **kva_pp);
LIBKV_API int32_t kvarena_get(
    const KVArena      *kva_p,
    uint32_t            ent_idx,
    KVArenaEntryView   *out_entview_p
);
LIBKV_API int kvarena_is_entry_valid(
    const KVArena  *kva_p,
    uint32_t        ent_idx
);

LIBKV_API uint32_t kvarena_size(KVArena *kva_p);
LIBKV_API uint32_t kvarena_delcnt(KVArena *kva_p);
LIBKV_API uint32_t kvarena_capacity(KVArena *kva_p);
LIBKV_API uint32_t kvarena_data_len(KVArena *kva_p);
LIBKV_API uint32_t kvarena_data_size(KVArena *kva_p);
LIBKV_API int kvarena_is_full(KVArena *kva_p);
LIBKV_API int kvarena_is_empty(KVArena *kva_p);
LIBKV_API void *kvarena_data(KVArena *kva_p);
LIBKV_API void *kvarena_entrytbl(KVArena *kva_p);

LIBKV_API int32_t kvarena_push_auto_grow(
    KVArena            *kva_p,
    const kvht_key_t   *kvht_key_p,
    const void         *data,
    uint32_t            data_len
);

LIBKV_API const KVArenaEntry *kvarena_get_entry(
    const KVArena  *kva_p,
    uint32_t        ent_idx
);
LIBKV_API const KVArenaEntryView *kvarena_entry_to_entview(
    const KVArena      *kva_p,
    const KVArenaEntry *ent_p,
    KVArenaEntryView   *out_entview_p
);

LIBKV_API KVArenaIterator kvarena_iterator_begin(const KVArena *kva_p);
LIBKV_API KVArenaIterator kvarena_iterator_end(const KVArena *kva_p);
LIBKV_API KVArenaIterator kvarena_iterator_next(
    const KVArena      *kva_p,
    KVArenaIterator     it
);

LIBKV_API KVArenaIterator kvarena_iterator_rbegin(const KVArena *kva_p);
LIBKV_API KVArenaIterator kvarena_iterator_rend(const KVArena *kva_p);
LIBKV_API KVArenaIterator kvarena_iterator_rnext(
    const KVArena      *kva_p,
    KVArenaIterator     it
);

#define kvarena_iterator_deref(kva_p, iter, out_entview_p) \
    kvarena_entry_to_entview(kva_p, iter, out_entview_p)


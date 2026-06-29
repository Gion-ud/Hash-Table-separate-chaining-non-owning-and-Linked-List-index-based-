#pragma once

#include "kvfile_def.h"
#include <_kvapi.h>
#include <kvht.h>
#include <kvarena.h>
#include <stdint.h>
#include <stddef.h>

typedef struct KVTable KVTable;
typedef KVArenaEntry KVTableEntry;
typedef KVArenaEntryView KVTableEntryView;
typedef KVArenaIterator KVTableIterator;

struct KVTable {
    kvht_t     *ht_p;
    KVArena    *kva_p;
    uint32_t    size;
    uint32_t    delcnt;
    uint32_t    capacity;
};

LIBKV_API KVTable *Create_KVTable(uint32_t entry_capacity);
LIBKV_API void Destroy_KVTable(KVTable *kvtbl_p);
LIBKV_API int KVTable_Insert(
    KVTable            *kvtbl_p,
    const kvht_key_t   *kvht_key_p,
    const void         *data,
    uint32_t            data_len
);
LIBKV_API int KVTable_Remove(
    KVTable            *kvtbl_p,
    const kvht_key_t   *kvht_key_p
);
LIBKV_API const KVArenaEntry *KVTable_GetEntry(
    const KVTable      *kvtbl_p,
    const kvht_key_t   *kvht_key_p
);
LIBKV_API const KVTableEntryView *KVTable_GetEntryView(
    const KVTable      *kvtbl_p,
    const kvht_key_t   *kvht_key_p,
    KVTableEntryView   *out_ev_p
);
LIBKV_API int KVTable_Compact(
    KVTable    *kvtbl_p
);
LIBKV_API int KVTable_BuildKVImageBuffer(
    const KVTable  *kvtbl_p,
    unsigned char **out_filebuf_pp,
    size_t         *out_filesize_p
);
LIBKV_API void KVTable_DestroyKVImageBuffer(
    const KVTable  *kvtbl_p,
    unsigned char **out_filebuf_pp
);

LIBKV_INLINED KVTableIterator
KVTableIterator_Begin(const KVTable *kvtbl_p) {
    return (!kvtbl_p) ? NULL : kvarena_iterator_begin(kvtbl_p->kva_p);
}
LIBKV_INLINED KVTableIterator
KVTableIterator_End(const KVTable *kvtbl_p) {
    return (!kvtbl_p) ? NULL : kvarena_iterator_end(kvtbl_p->kva_p);
}
LIBKV_INLINED KVTableIterator
KVTableIterator_Next(
    const KVTable      *kvtbl_p,
    KVTableIterator     iter
) {
    return (!kvtbl_p) ? NULL : kvarena_iterator_next(kvtbl_p->kva_p, iter);
}

LIBKV_INLINED const KVTableEntryView *KVTableIterator_Deref(
    const KVTable          *kvtbl_p,
    const KVTableIterator   iter,
    KVTableEntryView       *out_entview_p
) {
    return (!kvtbl_p) ? NULL : 
        kvarena_entry_to_entview(kvtbl_p->kva_p, iter, out_entview_p);
}

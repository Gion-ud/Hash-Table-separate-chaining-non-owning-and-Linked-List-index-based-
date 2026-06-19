#pragma once

#include <_kvapi.h>
#include <kvht.h>
#include <kvimg.h>
#include <kvfile.h>
#include <kvarena.h>
#include <stdint.h>
#include <stddef.h>

typedef struct KVTable KVTable;
typedef KVArenaEntry KVTableEntry;
typedef KVArenaEntryView KVTableEntryView;

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


#pragma once

#include "_kvapi.h"

#include <kvfile.h>
#include <kvarena.h>

LIBKV_API int kvarena_build_memimg_buf(
    KVArena        *kva_p,
    unsigned char **out_filebuf_pp,
    size_t         *out_filesize_p
);

LIBKV_API void kvarena_destroy_memimg_buf(
    KVArena        *kva_p,
    unsigned char **filebuf_pp
);

static inline const KVArenaEntryView *
KVFileReader_EntryViewFromFileEntry(
    const KVFile       *kvf_p,
    const KVFileEntry  *kvfent_p,
    KVArenaEntryView   *out_entview_p
) {
    if (!kvf_p || !kvfent_p || !out_entview_p) return NULL;

    out_entview_p->key_hash = kvfent_p->key_hash;
    out_entview_p->key_len  = kvfent_p->key_len;
    out_entview_p->key_p    = (const char*)kvf_p->data_p + kvfent_p->key_off;
    out_entview_p->val_len  = kvfent_p->val_len;
    out_entview_p->val_p    = (const void*)kvf_p->data_p + kvfent_p->val_off;

    return out_entview_p;
}

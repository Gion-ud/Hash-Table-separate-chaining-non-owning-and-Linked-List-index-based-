#pragma once

#include "_kvapi.h"
#include "kvfile_def.h"


typedef struct KVFileView {
    unsigned char  *buf_base;
    unsigned char  *buf_end;
    KVFileHeader   *header_p;
    KVFileEntry    *entrytbl;
    void           *data_p;
    KVFileFooter   *footer_p;
    uint32_t        align;
    uint32_t        entrycnt;
    uint32_t        data_len;
} KVFileView;

LIBKV_API int KVFile_Init(KVFile *kvf_p);
LIBKV_API void KVFile_Fini(KVFile *kvf_p);

LIBKV_API int KVFileReader_MapFile(KVFile *kvf_p, int fd);
LIBKV_API int KVFileReader_UnmapFile(KVFile *kvf_p);
LIBKV_API const KVFileEntry *KVFileReader_GetFileEntryChked(KVFile *kvf_p, uint32_t ent_idx);
static inline KVFileEntry *KVFileReader_GetFileEntry(KVFile *kvf_p, uint32_t ent_idx) {
    return &kvf_p->entrytbl[ent_idx];
}


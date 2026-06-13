#pragma once

#include "_kvapi.h"
#include "kvfile_def.h"

LIBKV_API int KVFile_CreateBuilderBuffer(
    KVFile     *kvf_p,
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
);
LIBKV_API void KVFile_DestroyBuilderBuffer(KVFile *kvf_p);

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

LIBKV_API KVFile *Create_KVFile();
LIBKV_API void Destroy_KVFile(KVFile *kvf_p);
LIBKV_API int KVFile_Init(KVFile *kvf_p);
LIBKV_API void KVFile_Fini(KVFile *kvf_p);
LIBKV_API const KVFileHeader *KVFileBuilder_WriteFileHeader(KVFile *kvf_p);
LIBKV_API const KVFileEntry *KVFileBuilder_WriteEntryTable(KVFile *kvf_p, KVFileEntry *entrytbl);
LIBKV_API const unsigned char *KVFileBuilder_WriteDataSection(KVFile *kvf_p, void *data_p);

LIBKV_API const unsigned char *KVFileBuilder_DataBufferBase(KVFile *kvf_p);
LIBKV_API const unsigned char *KVFileBuilder_DataBufferEnd(KVFile *kvf_p);

LIBKV_API const KVFileFooter *KVFileBuilder_WriteFileFooter(KVFile *kvf_p);
LIBKV_API int KVFileReader_MapFile(KVFile *kvf_p, int fd);
LIBKV_API int KVFileReader_UnmapFile(KVFile *kvf_p);

LIBKV_API const KVFileEntry *KVFileReader_GetFileEntryChked(KVFile *kvf_p, uint32_t ent_idx);
static inline KVFileEntry *KVFileReader_GetFileEntry(KVFile *kvf_p, uint32_t ent_idx) {
    return &kvf_p->entrytbl[ent_idx];
}


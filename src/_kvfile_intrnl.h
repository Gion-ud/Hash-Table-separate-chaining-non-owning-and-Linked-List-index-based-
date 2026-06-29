#pragma once

#include "_libkv_intrnl.h"
#include "kvfile_def.h"

LIBKV_INTRNL_API int _KVFile_CreateBuilderBuffer(
    KVFile     *kvf_p,
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
);
LIBKV_INTRNL_API void _KVFile_DestroyBuilderBuffer(KVFile *kvf_p);

LIBKV_INTRNL_API const KVFileHeader *_KVFileBuilder_WriteFileHeader(KVFile *kvf_p);
LIBKV_INTRNL_API const KVFileEntry *_KVFileBuilder_WriteEntryTable(KVFile *kvf_p, KVFileEntry *entrytbl);
LIBKV_INTRNL_API const unsigned char *_KVFileBuilder_WriteDataSection(KVFile *kvf_p, void *data_p);

LIBKV_INTRNL_API const unsigned char *_KVFileBuilder_DataBufferBase(KVFile *kvf_p);
LIBKV_INTRNL_API const unsigned char *_KVFileBuilder_DataBufferEnd(KVFile *kvf_p);

LIBKV_INTRNL_API const KVFileFooter *_KVFileBuilder_WriteFileFooter(KVFile *kvf_p);


#pragma once

#include "kvfile_def.h"

extern int KVFile_CreateBuilderBuffer(
    KVFile     *kvf_p,
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
);

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

extern void KVFile_DestroyBuilderBuffer(KVFile *kvf_p);
extern KVFile *Create_KVFile();
extern void Destroy_KVFile(KVFile *kvf_p);
extern int KVFile_Init(KVFile *kvf_p);
extern void KVFile_Fini(KVFile *kvf_p);
extern const KVFileHeader *KVFileBuilder_WriteFileHeader(KVFile *kvf_p);
extern const KVFileEntry *KVFileBuilder_WriteEntryTable(KVFile *kvf_p, KVFileEntry *entrytbl);
extern const unsigned char *KVFileBuilder_WriteDataSection(KVFile *kvf_p, void *data_p);

extern const unsigned char *KVFileBuilder_DataBufferBase(KVFile *kvf_p);
extern const unsigned char *KVFileBuilder_DataBufferEnd(KVFile *kvf_p);

extern const KVFileFooter *KVFileBuilder_WriteFileFooter(KVFile *kvf_p);
extern int KVFileReader_MapFile(KVFile *kvf_p, int fd);
extern int KVFileReader_UnmapFile(KVFile *kvf_p);

extern const KVFileEntry *KVFileReader_GetFileEntryChked(KVFile *kvf_p, uint32_t ent_idx);
static inline KVFileEntry *KVFileReader_GetFileEntry(KVFile *kvf_p, uint32_t ent_idx) {
    return &kvf_p->entrytbl[ent_idx];
}


/* To be done */

#include <kvfile.h>
#include <stdlib.h>
#include "dbg_print.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "alignoff.h"
#include <compute_crc32.h>

#define KV_FILE_MAGIC 0x46564BEFu
#define KV_FILE_EOF_MARKER 0x464F452Eu
#define KV_FILE_VERSION 0x0101u
#define KV_FILE_ALIGN 4u
#define KV_FILE_FLAGS (1u << 0)


KVFile *Create_KVFile() {
    KVFile *kvf_p = (KVFile*)malloc(sizeof(KVFile));
    if (!kvf_p) goto failed_ret;
    memset(kvf_p, 0, sizeof(*kvf_p));
    return kvf_p;
failed_ret:
    return NULL;
}
void Destroy_KVFile(KVFile *kvf_p) {
    if (kvf_p) free(kvf_p);
}

int KVFile_Init(KVFile *kvf_p) {
    if (!kvf_p) goto failed_ret;
    memset(kvf_p, 0, sizeof(*kvf_p));
    return 0;
failed_ret:
    return -1;
}
void KVFile_Fini(KVFile *kvf_p) {
    if (kvf_p) memset(kvf_p, 0, sizeof(*kvf_p));
}

static inline uint32_t
_KVFileBuilder_RequiredBufferSize(
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
) {
    return
        align_off(sizeof(KVFileHeader), align) +
        align_off(sizeof(KVFileEntry) * entrycnt, align) +
        align_off(data_len, align) +
        align_off(sizeof(KVFileFooter), align);
}

int KVFile_CreateBuilderBuffer(
    KVFile     *kvf_p,
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
) {
    if (!kvf_p || !data_len || !entrycnt) goto failed_ret;
    if (!is_pow2(align)) align = KV_FILE_ALIGN;

    uint32_t bufsize = _KVFileBuilder_RequiredBufferSize(data_len, entrycnt, align);
    _dbg_print(
        "KVFile_CreateBuilderBuffer@1._KVFileBuilder_RequiredBufferSize # mem_size_required: %u",
        bufsize
    );
    kvf_p->buf_base = (unsigned char*)malloc(bufsize);
    if (!kvf_p->buf_base) goto failed;
    memset(kvf_p->buf_base, 0, bufsize);
    kvf_p->buf_end = kvf_p->buf_base + bufsize;

    KVFileHeader *hdr_p = (KVFileHeader*)kvf_p->buf_base;
    KVFileEntry *etbl_p = (KVFileEntry*)(kvf_p->buf_base + align_off(sizeof(KVFileHeader), align));
    unsigned char *data_p = 
        kvf_p->buf_base +
        align_off(sizeof(KVFileHeader), align) +
        align_off(sizeof(KVFileEntry) * entrycnt, align);
    KVFileFooter *ftr_p = (KVFileFooter*)(
        kvf_p->buf_base +
        align_off(sizeof(KVFileHeader), align) +
        align_off(sizeof(KVFileEntry) * entrycnt, align) +
        align_off(data_len, align)
    );

    kvf_p->header_p     = hdr_p;
    kvf_p->entrytbl     = etbl_p;
    kvf_p->data_p       = data_p;
    kvf_p->footer_p     = ftr_p;
    kvf_p->align        = align;
    kvf_p->entrycnt     = entrycnt;
    kvf_p->data_len     = data_len;

    _dbg_print("KVFile_CreateBuilderBuffer@0.ret");
    return 0;
failed:
    _dbg_print("KVFile_CreateBuilderBuffer@-1.failed\n");
    KVFile_DestroyBuilderBuffer(kvf_p);
failed_ret:
    _dbg_print("KVFile_CreateBuilderBuffer@-1.failed_ret\n");
    return -1;
}

void KVFile_DestroyBuilderBuffer(KVFile *kvf_p) {
    _dbg_print("KVFile_DestroyBuilderBuffer@0\n");
    if (!kvf_p || !kvf_p->buf_base) goto scope_end;
    if (kvf_p->buf_base) free(kvf_p->buf_base);
    memset(kvf_p, 0, sizeof(*kvf_p));
scope_end:
    _dbg_print("KVFile_DestroyBuilderBuffer@exit\n");
}

const KVFileHeader *KVFileBuilder_WriteFileHeader(KVFile *kvf_p) {
    _dbg_print("KVFileBuilder_WriteFileHeader@0");
    assert(kvf_p);
    assert(kvf_p->header_p);
    if (!kvf_p || !kvf_p->header_p) goto failed;
    kvf_p->header_p->magic          = KV_FILE_MAGIC;
    kvf_p->header_p->version        = KV_FILE_VERSION;
    kvf_p->header_p->flags          = KV_FILE_FLAGS;
    kvf_p->header_p->align          = KV_FILE_ALIGN;
    kvf_p->header_p->entrycnt       = kvf_p->entrycnt;
    kvf_p->header_p->entrytbloff    = (uint32_t)((unsigned char*)kvf_p->entrytbl - kvf_p->buf_base);
    kvf_p->header_p->dataoff        = (uint32_t)((unsigned char*)kvf_p->data_p - kvf_p->buf_base);
    kvf_p->header_p->footeroff      = (uint32_t)((unsigned char*)kvf_p->footer_p - kvf_p->buf_base);
    kvf_p->header_p->eofoff         = (uint32_t)((unsigned char*)kvf_p->buf_end - kvf_p->buf_base);

    _dbg_print("KVFileBuilder_WriteFileHeader@0.ret\n");
    return kvf_p->header_p;
failed:
    _dbg_print("KVFileBuilder_WriteFileHeader@-1.failed.ret\n");
    return NULL;
}

const KVFileEntry *KVFileBuilder_WriteEntryTable(KVFile *kvf_p, KVFileEntry *entrytbl) {
    _dbg_print("KVFileBuilder_WriteEntryTable@0");
    if (!kvf_p || !kvf_p->entrytbl) goto failed;
    memcpy(kvf_p->entrytbl, entrytbl, kvf_p->entrycnt * sizeof(KVFileEntry));
    _dbg_print("KVFileBuilder_WriteEntryTable@0.ret\n");
    return kvf_p->entrytbl;
failed:
    _dbg_print("KVFileBuilder_WriteEntryTable@-1.failed.ret\n");
    return NULL;
}

const unsigned char *KVFileBuilder_WriteDataSection(KVFile *kvf_p, void *data_p) {
    _dbg_print("KVFileBuilder_WriteDataSection@0");
    if (!kvf_p || !kvf_p->data_p) goto failed;
    memcpy(kvf_p->data_p, data_p, kvf_p->data_len);
    _dbg_print("KVFileBuilder_WriteDataSection@0.ret\n");
    return kvf_p->data_p;
failed:
    _dbg_print("KVFileBuilder_WriteDataSection@-1.failed.ret\n");
    return NULL;
}

const KVFileFooter *KVFileBuilder_WriteFileFooter(KVFile *kvf_p) {
    _dbg_print("KVFileBuilder_WriteFileFooter@0");
    if (!kvf_p || !kvf_p->footer_p) goto failed;
    uint32_t footeroff = (uint32_t)(
        kvf_p->buf_end - kvf_p->buf_base -
        (ptrdiff_t)align_off(sizeof(KVFileFooter), kvf_p->align)
    );
    kvf_p->footer_p->crc32      = compute_mem_crc32(kvf_p->buf_base, footeroff);
    kvf_p->footer_p->eof_marker = KV_FILE_EOF_MARKER;

    _dbg_print("KVFileBuilder_WriteFileFooter@0.ret\n");
    return kvf_p->footer_p;
failed:
    _dbg_print("KVFileBuilder_WriteFileFooter@-1.failed.ret\n");
    return NULL;
}

const unsigned char *KVFileBuilder_DataBufferBase(KVFile *kvf_p) {
    return (!kvf_p || !kvf_p->buf_base) ? NULL : kvf_p->buf_base;
}
const unsigned char *KVFileBuilder_DataBufferEnd(KVFile *kvf_p) {
    return (!kvf_p || !kvf_p->buf_end) ? NULL : kvf_p->buf_end;
}

int KVFileReader_MapFile(KVFile *kvf_p, int fd) {
    _dbg_print("KVFileReader_MapFile@0");
    if (!kvf_p || fd < 0) goto failed_ret;

    struct stat st = {0};
    _dbg_print("KVFileReader_MapFile@1.fstat");
    if (fstat(fd, &st) < 0) goto failed_ret;

    uint32_t buf_size = st.st_size;
    _dbg_print("KVFileReader_MapFile@2.mmap");
    kvf_p->buf_base =
        (unsigned char*)mmap(
            NULL,
            buf_size,
            PROT_READ,
            MAP_SHARED | MAP_FILE,
            fd,
            0
        );
    if (kvf_p->buf_base == MAP_FAILED) goto failed_ret;

    _dbg_print("KVFileReader_MapFile@3.KVFileHeader");
    KVFileHeader *hdr_p = (KVFileHeader*)kvf_p->buf_base;
    assert(hdr_p->entrycnt);
    if (
        hdr_p->magic != KV_FILE_MAGIC ||
        hdr_p->version != KV_FILE_VERSION ||
        !is_pow2(hdr_p->align) ||
        !hdr_p->entrycnt
    ) goto failed;
    kvf_p->header_p = hdr_p;

    _dbg_print("KVFileReader_MapFile@4.KVFileEntry");
    KVFileEntry *etbl_p = (KVFileEntry*)(kvf_p->buf_base + hdr_p->entrytbloff);
    uint32_t _etbloff = align_off(sizeof(KVFileHeader), hdr_p->align);
    uint32_t _etblsize = align_off(hdr_p->entrycnt * sizeof(KVFileEntry), hdr_p->align);

    assert(is_aligned_off(hdr_p->entrytbloff, hdr_p->align));
    assert(hdr_p->entrytbloff < buf_size);
    assert(hdr_p->entrytbloff == _etbloff);
    assert(hdr_p->dataoff - hdr_p->entrytbloff == _etblsize);

    if (
        !is_aligned_off(hdr_p->entrytbloff, hdr_p->align) ||
        hdr_p->entrytbloff > buf_size ||
        hdr_p->entrytbloff != _etbloff ||
        hdr_p->dataoff - hdr_p->entrytbloff != _etblsize
    ) goto failed;

    _dbg_print("KVFileReader_MapFile@5.data");
    unsigned char *data_p = kvf_p->buf_base + hdr_p->dataoff;
    uint32_t _dataoff = _etbloff + _etblsize;
    uint32_t _datasize = hdr_p->footeroff - _dataoff;
    if (
        !is_aligned_off(hdr_p->dataoff, hdr_p->align) ||
        hdr_p->dataoff > buf_size ||
        hdr_p->dataoff != _dataoff
    ) goto failed;

    _dbg_print("KVFileReader_MapFile@6.footer");
    KVFileFooter *ftr_p = (KVFileFooter*)(kvf_p->buf_base + hdr_p->footeroff);
    if (
        !is_aligned_off(hdr_p->footeroff, hdr_p->align) || 
        hdr_p->footeroff > buf_size ||
        hdr_p->footeroff + sizeof(KVFileFooter) > hdr_p->eofoff ||
        hdr_p->eofoff != buf_size
    ) goto failed;
    _dbg_print("KVFileReader_MapFile@6.footer.crc");

    if (
        ftr_p->eof_marker != KV_FILE_EOF_MARKER ||
        ftr_p->crc32 != compute_mem_crc32(kvf_p->buf_base, hdr_p->footeroff)
    ) goto failed;

    _dbg_print("KVFileReader_MapFile@7");
    kvf_p->buf_end  = kvf_p->buf_base + st.st_size;
    kvf_p->entrytbl = etbl_p;
    kvf_p->data_p   = data_p;
    kvf_p->footer_p = ftr_p;
    kvf_p->align    = hdr_p->align;
    kvf_p->entrycnt = hdr_p->entrycnt;
    kvf_p->data_len = _datasize;

    _dbg_print("KVFileReader_MapFile@0.ret\n");
    return 0;
failed:
    munmap(kvf_p->buf_base, buf_size);
failed_ret:
    _dbg_print("KVFileReader_MapFile@-1.failed_ret\n");
    return -1;
}

int KVFileReader_UnmapFile(KVFile *kvf_p) {
    _dbg_print("KVFileReader_UnmapFile@0");
    if (!kvf_p || !kvf_p->buf_base || kvf_p->buf_base == MAP_FAILED) goto failed;

    _dbg_print("KVFileReader_UnmapFile@0.ret.munmap\n");
    return munmap(kvf_p->buf_base, kvf_p->buf_end - kvf_p->buf_base);    
failed:
    _dbg_print("KVFileReader_UnmapFile@-1.failed.ret\n");
    return -1;
}
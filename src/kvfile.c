/* To be done */

#include <kvfile.h>
#define _MEM_STREAM_IMPLM
#include <mem_stream.h>
#include <stdlib.h>
#include <dbg_print.h>
#include <string.h>
#include <sys/mman.h>
#include <alignoff.h>

#define KV_FILE_MAGIC 0x46564BEFu
#define KV_FILE_EOF_MARKER 0x464F452Eu
#define KV_FILE_VERSION 0x0101u
#define KV_FILE_ALIGN 4u
#define KV_FILE_FLAGS (1u << 0)

typedef struct KVFileStream {
    mem_stream_t   *stream_p;
    KVFileHeader   *header_p;
    KVFileEntry    *entrytbl_p;
    void           *data_p;
    KVFileFooter   *footer_p;
    uint32_t        align;
    uint32_t        entrycnt;
    uint32_t        data_len;
} KVFileStream;

static inline uint32_t
_KVFile_stream_size_required(
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

static inline KVFileHeader *
_KVFileStream_GetFileHeader(
    KVFileStream   *kvfs_p
) {
    return kvfs_p->stream_p->buf;
}

static inline KVFileEntry *
_KVFileStream_GetFileEntryTable(
    KVFileStream   *kvfs_p,
    uint32_t        align
) {
    return
        kvfs_p->stream_p->buf + 
        align_off(sizeof(KVFileHeader), align);
}

static inline unsigned char *
_KVFileStream_GetFileDataSection(
    KVFileStream   *kvfs_p,
    uint32_t        entrycnt,
    uint32_t        align
) {
    return
        kvfs_p->stream_p->buf +
        align_off(sizeof(KVFileHeader), align) +
        align_off(sizeof(KVFileEntry) * entrycnt, align);
}

static inline KVFileFooter *
_KVFileStream_GetFileFooter(
    KVFileStream   *kvfs_p,
    uint32_t        entrycnt,
    uint32_t        data_len,
    uint32_t        align
) {
    return
        kvfs_p->stream_p->buf +
        align_off(sizeof(KVFileHeader), align) +
        align_off(sizeof(KVFileEntry) * entrycnt, align) +
        align_off(data_len, align);
}



extern void Destroy_KVFileStream(KVFileStream *kvfs_p);
KVFileStream *Create_KVFileStream(
    uint32_t    data_len,
    uint32_t    entrycnt,
    uint32_t    align
) {
    if (!data_len || !entrycnt) goto failed_ret;
    if (!is_pow2(align)) align = KV_FILE_ALIGN;
    _dbg_print("Create_KVFileStream@0.malloc_KVFileStream");
    KVFileStream *kvfs_p = (KVFileStream*)malloc(sizeof(KVFileStream));
    if (!kvfs_p) goto failed_ret;
    __builtin_memset(kvfs_p, 0, sizeof(*kvfs_p));

    uint32_t stream_size =
        _KVFile_stream_size_required(data_len, entrycnt, align);
    _dbg_print(
        "Create_KVFileStream@1.create_mem_stream # mem_size_required: %u",
        stream_size
    );
    kvfs_p->stream_p = create_mem_stream(stream_size);
    if (!kvfs_p->stream_p) goto failed_ret;

    kvfs_p->header_p    = _KVFileStream_get_fileheader(kvfs_p);
    kvfs_p->entrytbl_p  = _KVFileStream_GetFileEntryTable(kvfs_p, align);
    kvfs_p->data_p      = _KVFileStream_GetFileDataSection(kvfs_p, entrycnt, align);
    kvfs_p->footer_p    = _KVFileStream_GetFileFooter(kvfs_p, entrycnt, data_len, align);
    kvfs_p->align       = align;
    kvfs_p->entrycnt    = entrycnt;
    kvfs_p->data_p      = data_len;

    _dbg_print("Create_KVFileStream@0.ret");
    return kvfs_p;
failed:
    _dbg_print("Create_KVFileStream@-1.failed");
    Destroy_KVFileStream(kvfs_p);
failed_ret:
    _dbg_print("Create_KVFileStream@-1.failed_ret\n");
    return NULL;
}




#pragma once
#include <stdint.h>
#include <assert.h>

typedef struct KVFileHeader {
    uint32_t    magic;          // [0]
    uint16_t    version;        // [1]
    uint16_t    flags;          // [2]
    uint32_t    align;          // [3]
    uint32_t    entrycnt;       // [4]
    uint32_t    entrytbloff;    // [5]
    uint32_t    dataoff;        // [6]
    uint32_t    footeroff;      // [7]
    uint32_t    eofoff;         // [8]
} KVFileHeader;

typedef struct KVFileFooter {
    uint32_t    crc32;      // [0]
    uint32_t    eof_marker; // [1]
} KVFileFooter;

typedef struct KVFileEntry {
    uint32_t    key_hash;   // [0]; hash32
    uint32_t    key_len;    // [1]
    uint32_t    key_off;    // [2]; offset to cstr key from the start of data section
    uint32_t    val_len;    // [3]
    uint32_t    val_off;    // [4]; offset to value blob from the start of data section
} KVFileEntry;


#define KV_FILE_HEADER_SIZE 32u
#define KV_FILE_FOOTER_SIZE 8u
#define KV_FILE_ENTRY_SIZE 20u

static_assert(
    sizeof(KVFileHeader) == KV_FILE_HEADER_SIZE,
    "KVFileHeader must be 32 bytes"
);
static_assert(
    sizeof(KVFileFooter) == KV_FILE_FOOTER_SIZE,
    "KVFileFooter must be 8 bytes"
);
static_assert(
    sizeof(KVFileEntry) == KV_FILE_ENTRY_SIZE,
    "KVFileEntry must be 20 bytes"
);

typedef struct KVFile {
    unsigned char  *buf_base;
    unsigned char  *buf_end;
    KVFileHeader   *header_p;
    KVFileEntry    *entrytbl;
    void           *data_p;
    KVFileFooter   *footer_p;
    uint32_t        align;
    uint32_t        entrycnt;
    uint32_t        data_len;
} KVFile;
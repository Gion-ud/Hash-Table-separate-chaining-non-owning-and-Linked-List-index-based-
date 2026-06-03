#pragma once

#include <stdint.h>
#include <assert.h>

#define KV_FILE_MAGIC 0x46564BEFu
#define KV_FILE_EOF_MARKER 0x464F452Eu
#define KV_FILE_VERSION 0x0101u
#define KV_FILE_HEADER_SIZE 32u
#define KV_FILE_FOOTER_SIZE 8u
#define KV_FILE_ENTRY_SIZE 20u
#define KV_FILE_ALIGN 4u
#define KV_FILE_FLAGS (1u << 0)

typedef struct KVFileHeader {
    uint32_t    magic;          // [0]
    uint16_t    version;        // [1]
    uint16_t    flags;          // [2]
    uint32_t    align;          // [3]
    uint32_t    entry_cnt;      // [4]
    uint32_t    entrytab_off;   // [5]
    uint32_t    data_off;       // [6]
    uint32_t    footer_off;     // [7]
    uint32_t    eof_off;        // [8]
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

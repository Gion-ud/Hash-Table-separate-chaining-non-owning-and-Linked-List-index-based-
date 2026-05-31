#ifndef HASH_FUNC_H
#define HASH_FUNC_H

#include <stddef.h>
#include <stdint.h>

static const uint64_t FNV_OFFSET_BASIS_UINT64   = 0xcbf29ce484222325ULL;
static const uint64_t FNV_PRIME_UINT64          = 0x00000100000001b3ULL;
static const uint32_t FNV_OFFSET_BASIS_UINT32   = 0x811C9DC5u;
static const uint32_t FNV_PRIME_UINT32          = 0x01000193u;


static inline uint64_t fnv_1a_hash64(const void* key, size_t len) {
    uint64_t h = FNV_OFFSET_BASIS_UINT64;
    size_t i = 0;
    while (i < len) {
        h ^= ((uint8_t*)key)[i++];
        h *= FNV_PRIME_UINT64;
    }
    return h;
}

static inline uint32_t fnv_1a_hash32(const void* key, size_t len) {
    uint32_t h = FNV_OFFSET_BASIS_UINT32;
    size_t i = 0;
    while (i < len) {
        h ^= ((uint8_t*)key)[i++];
        h *= FNV_PRIME_UINT32;
    }
    return h;
}

#endif
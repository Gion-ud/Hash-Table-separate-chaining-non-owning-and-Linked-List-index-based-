#pragma once

#include <zlib.h>
#include <stdint.h>

static inline uint32_t compute_mem_crc32(const void *buf, uint32_t buf_len) {
    if (!buf || !buf_len) return 0;
    uint32_t ulcrc32 = (uint32_t)crc32(0UL, Z_NULL, 0);
    ulcrc32 = (uint32_t)crc32((uLong)ulcrc32, (const Bytef*)buf, (uInt)buf_len);
    return ulcrc32;
}
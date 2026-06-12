#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct _chunk {
    uint32_t        len;
    unsigned char   data[];
} chunk_t;

typedef struct _buffer {
    uint32_t        len;
    unsigned char  *data;
} buffer_t;


#pragma once

#include <kvarena.h>
#include <kvfile.h>
#include "dbg_print.h"
#include "alignoff.h"
#include <hash_key.h>

struct KVArena {
    KVFileEntry    *entrytbl;       // [0]
    unsigned char  *data_buf;       // [1]
    uint8_t        *cntl_arr;       // [2]
    uint32_t        align;          // [3]
    uint32_t        entrycnt;       // [4]
    uint32_t        entrycap;       // [5]
    uint32_t        data_buf_len;   // [6]
    uint32_t        data_buf_size;  // [7]
};

static inline void _kva_assert_intrnl_state(KVArena *kva_p) {
    assert(kva_p->entrycap >= KVA_MIN_ENTC);
    assert(kva_p->data_buf_size >= KVA_MIN_BUFSIZE);
    assert(kva_p->entrycnt <= kva_p->entrycap);
    assert(kva_p->data_buf_len <= kva_p->data_buf_size);
    assert(is_pow2(kva_p->align));
    assert(kva_p->data_buf_len % kva_p->align == 0);
}

#pragma once

#include <kvht.h>
#include <kvimg.h>
#include <kvfile.h>
#include <kvarena.h>

typedef struct KVTable KVTable;


struct KVTable {
    kvht_t     *ht_p;
    KVArena    *kva_p;
};


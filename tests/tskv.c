#include "dbg_print.h"
#include <kvarena.h>
#include <kvfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <kvimg.h>
#include <kvht.h>
#define _USING_HASH_TABLE_UTILS


typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

extern int tskv_put_kv_loop(
    KVArena    *kva_p,
    uint32_t    kvcnt,
    cstr_kv_t  *kvarr
);
extern void tskv_get_kv_loop(
    KVArena    *kva_p
);
extern void tskv_erase_kv_ents(
    KVArena    *kva_p,
    uint32_t    delcnt,
    uint32_t   *ent_idx_arr
);
extern void tskv_compact(
    KVArena   **kva_pp
);


cstr_kv_t kvtbl[] = {
    {"open", "libc:fcntl.h:open"},
    {"close", "libc:unistd.h:close"},
    {"read", "libc:unistd.h:read"},
    {"write", "libc:unistd.h:write"},
    {"lseek", "libc:unistd.h:lseek"},
    {"mmap", "libc:sys/mman.h:mmap"},
    {"munmap", "libc:sys/mman.h:munmap"},
    {"ftruncate", "libc:unistd.h:ftruncate"},
    {"fsync", "libc:unistd.h:fsync"},
    {"msync", "libc:sys/mman.h:msync"},
    {"std::vector<int>::push_back", "libstdc++:vector:vector<int>::push_back"},
};
const uint32_t kvtbl_len = sizeof(kvtbl) / sizeof(*kvtbl);

int main() {
    KVArena *kva_p =
        create_kvarena(
            KVA_MIN_BUFSIZE,
            KVA_MIN_ENTC,
            KVA_ALIGN_DEFAULT
        );
    assert(kva_p);


    tskv_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tskv_get_kv_loop(kva_p);

    uint32_t entc = kvarena_size(kva_p);
    kvht_t *ht_p = create_kvht(entc, entc);
    assert(ht_p);

    puts("-----------------");
    for (uint32_t i = 0u; i < entc; ++i) {
        KVArenaEntryView ev = {0};
        printf("kv_put@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;
        kvht_key_t key = {
            .key        = ev.key_p,
            .key_len    = ev.key_len,
            .hash       = ev.key_hash
        };
        kvht_insert(ht_p, &key, &kvtbl[i]);
    }
    puts("-----------------");
    for (uint32_t i = 0u; i < entc; ++i) {
        KVArenaEntryView ev = {0};
        printf("kv_get@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;
        kvht_key_t key = {0};
        make_hash_key_from_cstr(&key, kvtbl[i].key);
        kvht_slot_handle_t h = {0};
        kvht_lookup(ht_p, &key, &h);
        kvht_slot_t *sp = kvht_get_slot(ht_p, &h);
        printf("%s -> %s\n", key.key, (char*)((cstr_kv_t*)sp->data)->value);
    }



    destroy_kvht(ht_p);

    unsigned char *buf = NULL;
    size_t buf_len = 0;
    int ret = kvarena_build_memimg_buf(kva_p, &buf, &buf_len);
    assert(ret >= 0);
    FILE *fp = fopen("data.bin", "wb+");
    assert(fp);
    size_t n = fwrite(buf, 1, buf_len, fp);
    assert(n == buf_len);
    fclose(fp);
    kvarena_destroy_memimg_buf(kva_p, &buf);

    destroy_kvarena(kva_p);


    KVFile *kvfp = Create_KVFile();
    int fd = open("data.bin", O_RDONLY);
    assert(fd != -1);
    ret = KVFileReader_MapFile(kvfp, fd);
    assert(ret >= 0);
    for (uint32_t i = 0u; i < kvfp->entrycnt; ++i) {
        const KVFileEntry *ep = KVFileReader_GetFileEntryChked(kvfp, i);
        assert(ep);
        KVArenaEntryView ev = {0};
        KVFileReader_EntryViewFromFileEntry(kvfp, ep, &ev);
        printf("[%u] %s %.*s\n", i, ev.key_p, ev.val_len, (char*)ev.val_p);
    }
    KVFileReader_UnmapFile(kvfp);

    close(fd);
    Destroy_KVFile(kvfp);

    return 0;
}

#include "../src/alignoff.h"

int tskv_put_kv_loop(
    KVArena    *kva_p,
    uint32_t    kvcnt,
    cstr_kv_t  *kvarr
) {
    assert(kva_p && kvcnt && kvarr);
    puts("-- tskv_put_kv_loop --");
    for (uint32_t i = 0u; i < kvcnt; ++i) {
        kvht_key_t hk = {0};
        printf("kv_put@line%u.loop[%u]\n", __LINE__, i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = kvarena_push_auto_grow(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");
    return 0;
}


void tskv_get_kv_loop(
    KVArena    *kva_p
) {
    assert(kva_p);
    puts("-- tskv_put_kv_loop --");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("kv_get@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;
        printf("[%u] key='%s'; val='%.*s'\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
}
void tskv_erase_kv_ents(
    KVArena    *kva_p,
    uint32_t    delcnt,
    uint32_t   *ent_idx_arr
) {
    assert(kva_p && delcnt && ent_idx_arr);
    puts("-- tskv_erase_kv_ents --");
    for (uint32_t i = 0u; i < delcnt; ++i) {
        printf("kv_erase@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_mark_dead(kva_p, ent_idx_arr[i]);
        if (ret < 0) {
            printf("kv_erase@loop[%u].ent[%u] failed\n", i, ent_idx_arr[i]);
        }
    }
    puts("");
}
void tskv_compact(
    KVArena   **kva_pp
) {
    assert(kva_pp && *kva_pp);
    puts("-- tskv_compact --");
    int ret = kvarena_compact(kva_pp);
    assert(ret >= 0);
    puts("");
}
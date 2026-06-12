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
extern void kvarena_dump_kvfile(
    KVArena    *kva_p,
    const char *filename
);
extern void kvarena_load_kvfile(
    KVArena    *kva_p,
    const char *filename
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

    uint32_t *delidx_arr = (uint32_t[]){0, 1, 3, 5, 7, 11, 13};
    tskv_erase_kv_ents(
        kva_p,
        7,
        delidx_arr
    );
    tskv_get_kv_loop(kva_p);
    tskv_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tskv_get_kv_loop(kva_p);

    delidx_arr = (uint32_t[]){2, 4, 6, 8, 10, 12, 16};
    tskv_erase_kv_ents(
        kva_p,
        7,
        delidx_arr
    );
    tskv_get_kv_loop(kva_p);
    tskv_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tskv_get_kv_loop(kva_p);
    tskv_compact(&kva_p);




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
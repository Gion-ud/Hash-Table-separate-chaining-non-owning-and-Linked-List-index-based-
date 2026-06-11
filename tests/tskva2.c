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

#ifdef _DEBUG
#include <compute_crc32.h>
#endif

typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

extern int tstkva_put_kv_loop(
    KVArena    *kva_p,
    uint32_t    kvcnt,
    cstr_kv_t  *kvarr
);
extern void tstkva_get_kv_loop(
    KVArena    *kva_p
);
extern void tstkva_erase_kv_ents(
    KVArena    *kva_p,
    uint32_t    delcnt,
    uint32_t   *ent_idx_arr
);
extern void tstkva_compact(
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


    tstkva_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tstkva_get_kv_loop(kva_p);

    uint32_t *delidx_arr = (uint32_t[]){0, 1, 3, 5, 7, 11, 13};
    tstkva_erase_kv_ents(
        kva_p,
        7,
        delidx_arr
    );
    tstkva_get_kv_loop(kva_p);
    tstkva_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tstkva_get_kv_loop(kva_p);

    delidx_arr = (uint32_t[]){2, 4, 6, 8, 10, 12, 16};
    tstkva_erase_kv_ents(
        kva_p,
        7,
        delidx_arr
    );
    tstkva_get_kv_loop(kva_p);
    tstkva_put_kv_loop(kva_p, kvtbl_len, kvtbl);
    tstkva_get_kv_loop(kva_p);
    tstkva_compact(&kva_p);
    kvarena_dump_kvfile(kva_p, "kv.bin");

    destroy_kvarena(kva_p);


    KVFile kvf = {0};
    KVFile_Init(&kvf);
    int fd = open("kv.bin", O_RDONLY);
    assert(fd != -1);
    int ret = KVFileReader_MapFile(&kvf, fd);
    assert(ret >= 0);

    KVFileHeader *hdr_p     = kvf.header_p;
    KVFileEntry *etbl_p     = kvf.entrytbl;
    unsigned char *data_p   = kvf.data_p;
    ptrdiff_t _fbufdiff = kvf.buf_end - kvf.buf_base;
    assert(_fbufdiff > 0);
    uint32_t filesize = (uint32_t)_fbufdiff;

    for (uint32_t i = 0u; i < hdr_p->entrycnt; ++i) {
        const KVFileEntry *ep = &etbl_p[i];
        assert(ep->key_off + ep->key_len + ep->val_len < filesize);
        assert(ep->key_off < ep->val_off);

        char *key_p = (char*)data_p + ep->key_off;
        char *val_p = (char*)data_p + ep->val_off;

        printf("[%u] %s %.*s\n", i, key_p, ep->val_len, val_p);
    }

    ret = KVFileReader_UnmapFile(&kvf);
    assert(ret >= 0);

    close(fd);
    KVFile_Fini(&kvf);

    return 0;
}

int tstkva_put_kv_loop(
    KVArena    *kva_p,
    uint32_t    kvcnt,
    cstr_kv_t  *kvarr
) {
    assert(kva_p && kvcnt && kvarr);
    puts("-- tstkva_put_kv_loop --");
    for (uint32_t i = 0u; i < kvcnt; ++i) {
        hash_key_t hk = {0};
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


void tstkva_get_kv_loop(
    KVArena    *kva_p
) {
    assert(kva_p);
    puts("-- tstkva_put_kv_loop --");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("kv_get@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;
        printf("[%u] key='%s'; val='%.*s'\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
}
void tstkva_erase_kv_ents(
    KVArena    *kva_p,
    uint32_t    delcnt,
    uint32_t   *ent_idx_arr
) {
    assert(kva_p && delcnt && ent_idx_arr);
    puts("-- tstkva_erase_kv_ents --");
    for (uint32_t i = 0u; i < delcnt; ++i) {
        printf("kv_erase@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_mark_dead(kva_p, ent_idx_arr[i]);
        if (ret < 0) {
            printf("kv_erase@loop[%u].ent[%u] failed\n", i, ent_idx_arr[i]);
        }
    }
    puts("");
}
void tstkva_compact(
    KVArena   **kva_pp
) {
    assert(kva_pp && *kva_pp);
    puts("-- tstkva_compact --");
    int ret = kvarena_compact(kva_pp);
    assert(ret >= 0);
    puts("");
}
void kvarena_dump_kvfile(
    KVArena    *kva_p,
    const char *filename
) {
    assert(kva_p && filename);
    puts("-- kvarena_dump_kvfile --");
    assert(kvarena_size(kva_p));
    KVFile kvf = {0};
    KVFile_Init(&kvf);
    int ret =
        KVFile_CreateBuilderBuffer(
            &kvf,
            kvarena_data_len(kva_p),
            kvarena_size(kva_p),
            KVA_ALIGN_DEFAULT
        );
    assert(ret >= 0);
    assert(kvf.entrycnt);

    const KVFileHeader *hdr_p = KVFileBuilder_WriteFileHeader(&kvf);
    assert(hdr_p);

    const KVFileEntry *etbl_p =
        KVFileBuilder_WriteEntryTable(
            &kvf,
            (KVFileEntry*)kvarena_entrytbl(kva_p)
        );
    assert(etbl_p);

    const unsigned char *data_p =
        KVFileBuilder_WriteDataSection(&kvf, kvarena_data(kva_p));
    assert(data_p);

    const KVFileFooter *ftr_p = KVFileBuilder_WriteFileFooter(&kvf);
    assert(ftr_p);

    const unsigned char *kvfbuf_base = KVFileBuilder_DataBufferBase(&kvf);
    const unsigned char *kvfbuf_end = KVFileBuilder_DataBufferEnd(&kvf);
    assert(kvfbuf_base);
    assert(kvfbuf_end);
    ptrdiff_t kvfbuf_len = kvfbuf_end - kvfbuf_base;
    assert(kvfbuf_len > 0);



    FILE *fp = fopen("kv.bin", "wb+");
    assert(fp);
    size_t n = fwrite(kvfbuf_base, 1, (size_t)kvfbuf_len, fp);
    assert(n == (size_t)kvfbuf_len);
    fclose(fp);

    KVFile_DestroyBuilderBuffer(&kvf);
    KVFile_Fini(&kvf);
    puts("");
}

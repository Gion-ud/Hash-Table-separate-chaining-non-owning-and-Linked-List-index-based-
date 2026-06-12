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

#ifdef _DEBUG
#include <compute_crc32.h>
#endif

typedef struct kv {
    const char *key;
    const char *value;
} kv_t;

kv_t kvtbl[] = {
    {"open", "libc::fcntl::open"},
    {"close", "libc::fcntl::close"},
    {"read", "libc::unistd::read"},
    {"write", "libc::unistd::write"},
    {"lseek", "libc::unistd::lseek"},
    {"mmap", "libc::sys_mman::mmap"},
    {"munmap", "libc::sys_mman::munmap"},
    {"ftruncate", "libc::unistd::ftruncate"},
    {"fsync", "libc::unistd::fsync"},
    {"msync", "libc::sys_mman::msync"},
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
    
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = kvarena_push_auto_grow(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");
    

    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        assert(ret != NULL_IDX);
        printf("key: %s; val: %.*s\n", ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");


    uint32_t kva_size = kvarena_size(kva_p);
    for (uint32_t i = 0u; i < kva_size; ++i) {
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_pop(kva_p);
        assert(ret >= 0);
    }
    puts("");

    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        assert(ret != NULL_IDX);
        printf("key: %s; val: %.*s\n", ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");

    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        puts("main@5.loop");
        int ret = kvarena_push(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");

    kvarena_mark_dead(kva_p, 1);
    kvarena_mark_dead(kva_p, 2);
    kvarena_mark_dead(kva_p, 3);
    kvarena_mark_dead(kva_p, 5);
    kvarena_mark_dead(kva_p, 7);
    kvarena_mark_dead(kva_p, 11);

    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = kvarena_push_auto_grow(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");

    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;;
        printf("[%u] key: %s; val: %.*s\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
    _dbg_print("main@4.loop.end\n");

    kvarena_compact(&kva_p);

    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        printf("main@line%u.loop[%u]\n", __LINE__, i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;;
        printf("[%u] key: %s; val: %.*s\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");

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
    ptrdiff_t kvfbuf_diff = kvfbuf_end - kvfbuf_base;
    assert(kvfbuf_diff > 0);



    FILE *fp = fopen("kv.bin", "wb+");
    assert(fp);
    size_t n = fwrite(kvfbuf_base, 1, (size_t)kvfbuf_diff, fp);
    assert(n == (size_t)kvfbuf_diff);
    fclose(fp);
    KVFile_DestroyBuilderBuffer(&kvf);

    KVFile_Fini(&kvf);
    destroy_kvarena(kva_p);



    KVFile_Init(&kvf);
    int fd = open("kv.bin", O_RDONLY);
    assert(fd != -1);
    ret = KVFileReader_MapFile(&kvf, fd);
    assert(ret >= 0);

    hdr_p = kvf.header_p;
    etbl_p = kvf.entrytbl;
    data_p = kvf.data_p;
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
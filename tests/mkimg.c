#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <kvtbl.h>
#include <kvht.h>
#include <kvimg.h>
#define _USING_HASH_TABLE_UTILS


typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

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
    {"malloc", "libc:stdlib.h:malloc"},
    {"calloc", "libc:stdlib.h:calloc"},
    {"realloc", "libc:stdlib.h:realloc"},
    {"free", "libc:stdlib.h:free"},
};
const uint32_t kvtbl_len = sizeof(kvtbl) / sizeof(*kvtbl);

static inline void tskv_print_ev(const KVTableEntryView *evp) {
    printf(
        "(0x%.8x, %u, '%.*s', %u, '%.*s')\n",
        evp->key_hash,
        evp->key_len,
        evp->key_len,evp->key_p,
        evp->val_len,
        evp->val_len,(char*)evp->val_p
    );
}

int main() {
    KVTable *kvtbl_p = Create_KVTable(1);
    assert(kvtbl_p);

    puts(" -- 1. insert all keys -- ");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        printf("@it %u\n", i);
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int rc = KVTable_Insert(
            kvtbl_p,
            &hk,
            kvtbl[i].value,
            strlen(kvtbl[i].value)
        );
        assert(rc != -1);
    }
    puts("");

    puts(" -- 2. get all kv -- ");

    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        KVTableEntryView ev = {0};
        KVTable_GetEntryView(kvtbl_p, &hk, &ev);
        tskv_print_ev(&ev);
    }
    puts("");
    printf("entc: %u\n", kvtbl_p->size);

    puts(" -- 3. get all kv -- ");
    for (
    __auto_type
        it = KVTableIterator_Begin(kvtbl_p); 
        it != KVTableIterator_End(kvtbl_p);
        it = KVTableIterator_Next(kvtbl_p, it)
    ) {
        kvht_key_t hk = {0};
        KVTableEntryView ev = {0};
        __auto_type __evp = KVTableIterator_Deref(kvtbl_p, it, &ev);
        make_hash_key_from_cstr(&hk, __evp->key_p);
        KVTable_GetEntryView(kvtbl_p, &hk, &ev);
        tskv_print_ev(&ev);
    }
    puts("");

    puts(" -- 14. serialise -- ");
    unsigned char *filebuf = NULL;
    size_t filesize = 0ul;
    int rc = KVTable_BuildKVImageBuffer(kvtbl_p, &filebuf, &filesize);
    assert(rc != -1);
    FILE *fp = fopen("data.bin", "wb");
    assert(fp);

    fwrite(filebuf, 1, filesize, fp);

    fclose(fp);
    KVTable_DestroyKVImageBuffer(kvtbl_p, &filebuf);
    Destroy_KVTable(kvtbl_p);

    puts(" -- 15. reload -- ");
    KVFile kvf = {0};
    fp = fopen("data.bin", "rb+");
    assert(fp);

    KVFileReader_MapFile(&kvf, fileno(fp));
    for (uint32_t i = 0u; i < kvf.entrycnt; ++i) {
        __auto_type ent_p = KVFileReader_GetFileEntryChked(&kvf, i);
        assert(ent_p);
        KVArenaEntryView ev = {0};
        __auto_type _evp = KVFileReader_EntryViewFromFileEntry(&kvf, ent_p, &ev);
        assert(_evp);
        printf("[%u] %s %.*s\n", i, ev.key_p, ev.val_len, (char*)ev.val_p);
    }

    kvtbl_p = Create_KVTable(1);
    puts(" -- 16. reload -- ");
    for (uint32_t i = 0u; i < kvf.entrycnt; ++i) {
        printf("@it %u\n", i);
        __auto_type ent_p = KVFileReader_GetFileEntryChked(&kvf, i);
        assert(ent_p);
        KVArenaEntryView ev = {0};
        __auto_type _evp = KVFileReader_EntryViewFromFileEntry(&kvf, ent_p, &ev);
        assert(_evp);
        tskv_print_ev(_evp);
        kvht_key_t hk = { ev.key_p, ev.key_len, ev.key_hash };
        int rc = KVTable_Insert(
            kvtbl_p,
            &hk,
            ev.val_p,
            ev.val_len
        );
        assert(rc != -1);
    }
    puts("");
    KVFileReader_UnmapFile(&kvf);

    puts(" -- 3. get all kv -- ");
    for (
    __auto_type
        it = KVTableIterator_Begin(kvtbl_p); 
        it != KVTableIterator_End(kvtbl_p);
        it = KVTableIterator_Next(kvtbl_p, it)
    ) {
        KVTableEntryView ev = {0};
        __auto_type __evp = KVTableIterator_Deref(kvtbl_p, it, &ev);
        assert(__evp);
        //tskv_print_ev(&ev);
        kvht_key_t hk = { ev.key_p, ev.key_len, ev.key_hash };
        __evp = KVTable_GetEntryView(kvtbl_p, &hk, &ev);
        assert(__evp);
        tskv_print_ev(__evp);
    }
    puts("");

    fclose(fp);

    Destroy_KVTable(kvtbl_p);
    return 0;
}

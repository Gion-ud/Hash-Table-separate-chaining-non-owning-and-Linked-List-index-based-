#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <kvtbl.h>
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

static inline void tskv_print_ev(KVTableEntryView *evp) {
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

    __auto_type kva_ent_tbl = (KVArenaEntry*)kvarena_entrytbl(kvtbl_p->kva_p);
    assert(kva_ent_tbl);
    puts(" -- 2. get all kvarena kv -- ");
    for (uint32_t i = 0u; i < kvtbl_p->size; ++i) {
        __auto_type kva_ent_p = &kva_ent_tbl[i];
        KVArenaEntryView ev = {0};
        __auto_type __evp = kvarena_entry_to_entview(kvtbl_p->kva_p, kva_ent_p, &ev);
        assert(__evp);
        tskv_print_ev(&ev);
        printf(
            "%s -> '%.*s'\t@it [%u]\n",
            ev.key_p, ev.val_len, (char*)ev.val_p, i
        );
    }
    puts("");

    puts(" -- 3. get all kv -- ");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        KVTableEntryView ev = {0};
        __auto_type __ep = KVTable_GetEntry(
            kvtbl_p,
            &hk
        );
        assert(__ep);
        //assert(__ep == &kva_ent_tbl[i]);
        __auto_type __evp = kvarena_entry_to_entview(
            kvtbl_p->kva_p,
            __ep,
            &ev
        );
        assert(__evp);
        tskv_print_ev(&ev);

        printf(
            "@it [%u] %s -> '%.*s'\n",
            i, kvtbl[i].key, ev.val_len, (char*)ev.val_p
        );
    }
    puts("");


    uint32_t kv_erase_idx_arr[] = {
        1, 3, 5, 7, 9, 11, 13
    };
    puts(" -- 4. erase some kv -- ");
    for (uint32_t i = 0u; i < 7; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[kv_erase_idx_arr[i]].key);
        __auto_type rc = KVTable_Remove(
            kvtbl_p,
            &hk
        );
        assert(rc != -1);

    }
    puts("");


    puts(" -- 5. lookup kv -- ");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        KVTableEntryView ev = {0};
        __auto_type __ep = KVTable_GetEntry(
            kvtbl_p,
            &hk
        );
        if (!__ep) continue;
        //assert(__ep == &kva_ent_tbl[i]);
        __auto_type __evp = kvarena_entry_to_entview(
            kvtbl_p->kva_p,
            __ep,
            &ev
        );
        assert(__evp);
        tskv_print_ev(&ev);

        printf(
            "@it [%u] %s -> '%.*s'\n",
            i, kvtbl[i].key, ev.val_len, (char*)ev.val_p
        );
    }
    puts("");


    puts(" -- 6. Compaction -- ");
    KVTable_Compact(kvtbl_p);


    puts(" -- 7. lookup kv after GC -- ");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        KVTableEntryView ev = {0};
        __auto_type __ep = KVTable_GetEntry(
            kvtbl_p,
            &hk
        );
        if (!__ep) continue;
        //assert(__ep == &kva_ent_tbl[i]);
        __auto_type __evp = kvarena_entry_to_entview(
            kvtbl_p->kva_p,
            __ep,
            &ev
        );
        assert(__evp);
        tskv_print_ev(&ev);

        printf(
            "@it [%u] %s -> '%.*s'\n",
            i, kvtbl[i].key, ev.val_len, (char*)ev.val_p
        );
    }
    puts("");
    printf("entc: %u\n", kvtbl_p->size);


    Destroy_KVTable(kvtbl_p);
    return 0;
}

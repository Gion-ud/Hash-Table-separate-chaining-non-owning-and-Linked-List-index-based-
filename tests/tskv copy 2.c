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

#define stack_alloc_arr(T, N) (T[N]){0}
#define heap_alloc(T) (T*)calloc(1, sizeof(T))
#define heap_alloc_arr(T, n) (T*)calloc(n, sizeof(T))
#define heap_free(ptr) free(ptr)
#define heap_new(T) heap_alloc(T)
#define heap_new_arr(T, n) heap_alloc_arr(T, n)



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
    KVArenaIterator it_begin = kvarena_iterator_begin(kvtbl_p->kva_p);
    KVArenaIterator it_end = kvarena_iterator_end(kvtbl_p->kva_p);
    KVArenaIterator it = {0};
    for (
        it = it_begin; it != it_end;
        it = kvarena_iterator_next(kvtbl_p->kva_p, it)
    ) {
        KVTableEntryView ev = {0};
        kvarena_entry_to_entview(kvtbl_p->kva_p, it, &ev);
        printf("[%td] ", it - it_begin);tskv_print_ev(&ev);
    }
    puts("");

    KVArenaIterator it_rbegin = kvarena_iterator_rbegin(kvtbl_p->kva_p);
    KVArenaIterator it_rend = kvarena_iterator_rend(kvtbl_p->kva_p);
    for (
        it = it_rbegin; it != it_rend;
        it = kvarena_iterator_rnext(kvtbl_p->kva_p, it)
    ) {
        KVTableEntryView ev = {0};
        kvarena_entry_to_entview(kvtbl_p->kva_p, it, &ev);
        printf("[%td] ", it - it_begin);tskv_print_ev(&ev);
    }
    puts("");

    puts(" -- 3. get all kv -- ");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        kvht_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        KVTableEntryView ev = {0};
        __auto_type __evp = KVTable_GetEntryView(
            kvtbl_p,
            &hk,
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
        __auto_type __evp = KVTable_GetEntryView(
            kvtbl_p,
            &hk,
            &ev
        );
        if (!__evp) {
            puts("lookup failed");
            continue;
        }
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
        __auto_type __evp = KVTable_GetEntryView(
            kvtbl_p,
            &hk,
            &ev
        );
        if (!__evp) {
            puts("lookup failed");
            continue;
        }
        tskv_print_ev(&ev);

        printf(
            "@it [%u] %s -> '%.*s'\n",
            i, kvtbl[i].key, ev.val_len, (char*)ev.val_p
        );
    }
    puts("");
    printf("entc: %u\n", kvtbl_p->size);

    int *p = stack_alloc_arr(int, 4);
    p[0] = 0;
    p[1] = 1;
    p[2] = 2;
    p[3] = 3;
    for (__auto_type i = 0u; i < 4; ++i) {
        printf("%d\n", p[i]);
    }
    



    Destroy_KVTable(kvtbl_p);
    return 0;
}

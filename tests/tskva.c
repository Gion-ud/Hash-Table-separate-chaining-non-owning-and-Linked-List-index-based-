#include "dbg_print.h"
#include "kvarena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

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
    _dbg_print("main@0.create_kvarena");
    KVArena *kva_p =
        create_kvarena(
            KVA_MIN_BUFSIZE,
            KVA_MIN_ENTC,
            KVA_ALIGN_DEFAULT
        );
    assert(kva_p);
    
    _dbg_print("main@1.loop.begin\n");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        _dbg_print("main@1.loop[%u].make_hash_key_from_cstr", i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        puts("main@1.loop");
        _dbg_print("main@1.loop[%u].kvarena_push_auto_grow", i);
        int ret = kvarena_push_auto_grow(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");
    _dbg_print("main@1.loop.end\n");
    

    _dbg_print("main@2.loop.begin\n");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        _dbg_print("main@2.loop[%u].kvarena_get", i);
        int ret = kvarena_get(kva_p, i, &ev);
        assert(ret != NULL_IDX);
        _dbg_print("main@2.loop[%u].deref", i);
        printf("key: %s; val: %.*s\n", ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
    _dbg_print("main@2.loop.end\n");

    _dbg_print("main@3.loop.begin\n");
    uint32_t kva_size = kvarena_size(kva_p);
    for (uint32_t i = 0u; i < kva_size; ++i) {
        hash_key_t hk = {0};
        _dbg_print("main@1.loop[%u].kvarena_pop", i);
        int ret = kvarena_pop(kva_p);
        assert(ret >= 0);
    }
    puts("");
    _dbg_print("main@3.loop.end\n");

    _dbg_print("main@4.loop.begin\n");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        _dbg_print("main@4.loop[%u].kvarena_get", i);
        int ret = kvarena_get(kva_p, i, &ev);
        assert(ret != NULL_IDX);
        _dbg_print("main@4.loop[%u].deref", i);
        printf("key: %s; val: %.*s\n", ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
    _dbg_print("main@4.loop.end\n");

    _dbg_print("main@5.loop.begin\n");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        _dbg_print("main@5.loop[%u].make_hash_key_from_cstr", i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        puts("main@5.loop");
        _dbg_print("main@5.loop[%u].kvarena_push", i);
        int ret = kvarena_push(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");
    _dbg_print("main@5.loop.end\n");

    kvarena_mark_dead(kva_p, 1);
    kvarena_mark_dead(kva_p, 2);
    kvarena_mark_dead(kva_p, 3);
    kvarena_mark_dead(kva_p, 5);
    kvarena_mark_dead(kva_p, 7);
    kvarena_mark_dead(kva_p, 11);

    _dbg_print("main@5.loop.begin\n");
    for (uint32_t i = 0u; i < kvtbl_len; ++i) {
        hash_key_t hk = {0};
        _dbg_print("main@5.loop[%u].make_hash_key_from_cstr", i);
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        puts("main@5.loop");
        _dbg_print("main@5.loop[%u].kvarena_push_auto_grow", i);
        int ret = kvarena_push_auto_grow(
            kva_p, &hk, kvtbl[i].value, strlen(kvtbl[i].value)
        );
        assert(ret >= 0);
    }
    puts("");
    _dbg_print("main@5.loop.end\n");

    _dbg_print("main@4.loop.begin\n");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        _dbg_print("main@4.loop[%u].kvarena_get", i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;;
        _dbg_print("main@4.loop[%u].deref", i);
        printf("[%u] key: %s; val: %.*s\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
    _dbg_print("main@4.loop.end\n");

    kvarena_compact(&kva_p);

    _dbg_print("main@4.loop.begin\n");
    for (uint32_t i = 0u; i < kvarena_size(kva_p); ++i) {
        KVArenaEntryView ev = {0};
        _dbg_print("main@4.loop[%u].kvarena_get", i);
        int ret = kvarena_get(kva_p, i, &ev);
        if (ret == NULL_IDX) continue;;
        _dbg_print("main@4.loop[%u].deref", i);
        printf("[%u] key: %s; val: %.*s\n", i, ev.key_p, (int)ev.val_len, (char*)ev.val_p);
    }
    puts("");
    _dbg_print("main@4.loop.end\n");

    _dbg_print("main@0.destroy_kvarena");
    destroy_kvarena(kva_p);
    _dbg_print("main@0.ret");
    return 0;
}
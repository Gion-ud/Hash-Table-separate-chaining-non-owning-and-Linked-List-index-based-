#include <hash_table.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hash_fn.h>

#define NBUCKETS 8
#define NSLOTS 8

typedef struct kv {
    const char *key;
    const char *value;
} kv_t;

kv_t kvtbl[] = {
    {"open", "fcntl.h::open"},
    {"close", "fcntl.h::close"},
    {"read", "io.h::read"},
    {"write", "io.h::write"},
    {"lseek", "io.h::lseek"},
    {"ioctl", "ioctl.h::ioctl"},
    {"mmap", "sys/mman.h::mmap"},
    {"mummap", "sys/mman.h::mummap"},
};

const uint32_t kvc = sizeof(kvtbl) / sizeof(*kvtbl);

#include <assert.h>

int main() {
    hash_table_t *ht_p = create_hash_table(NBUCKETS, NSLOTS);
    assert(ht_p);

    for (uint32_t i = 0u; i < kvc; ++i) {
        hash_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = hash_table_insert(ht_p, &hk, kvtbl[i].value);
        if (ret < 0) puts("hash_table_insert failed");
        else puts("hash_table_insert: success");
    }

    for (uint32_t i = 0u; i < kvc; ++i) {
        hash_slot_handle_t hsh = { NULL_IDX, NULL_IDX };
        hash_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = hash_table_lookup(ht_p, &hk, &hsh);
        if (ret < 0) {
            puts("hash_table_lookup failed: key not found");
            continue;
        }
        const hash_slot_t *slot_p = hash_table_get_slot(ht_p, &hsh);
        assert(slot_p);
        printf("%s\n", (char*)slot_p->data);
        hash_table_destroy_handle(ht_p, &hsh);
    }


    destroy_hash_table(ht_p);
    return 0;
}
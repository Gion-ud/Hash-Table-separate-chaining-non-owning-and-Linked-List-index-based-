#include <hash_table.h>
#define _USING_HASH_TABLE_UTILS
#include <ht_utils.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NBUCKETS 4
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
    {"munmap", "sys/mman.h::mummap"},
};

const uint32_t kvc = sizeof(kvtbl) / sizeof(*kvtbl);

#include <assert.h>

int main() {
    hash_table_t *ht_p = create_hash_table(NBUCKETS, NSLOTS);
    assert(ht_p);

    puts("- 1. insert all keys");
    for (uint32_t i = 0u; i < kvc; ++i) {
        hash_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = hash_table_insert(ht_p, &hk, kvtbl[i].value);
        if (ret < 0) puts("hash_table_insert failed");
        else puts("hash_table_insert: success");
    }
    puts("");

    puts("- 2. lookup all keys");
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
        ht_slot_handle_reset(&hsh);
    }
    puts("");

    puts("- 3. test ht_get convience macro");
    printf("%s\n", (char*)ht_get(ht_p, "read")); // macro for convience

    puts("- 4. test ht_erase convience macro key='ioctl'");
    int ret = ht_erase(ht_p, "ioctl"); // macro for convience
    assert(ret >= 0);

    puts("- 5. iterate over to assure that key is deleted and - 6. delete each entry");
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
        hash_table_remove(ht_p, &hsh);
        ht_slot_handle_reset(&hsh);
    }
    puts("");

    puts("- 6. lookup all keys to ensure each is erased");
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
        ht_slot_handle_reset(&hsh);
    }
    puts("");

    puts("- 7. reinsert all keys");
    for (uint32_t i = 0u; i < kvc; ++i) {
        hash_key_t hk = {0};
        make_hash_key_from_cstr(&hk, kvtbl[i].key);
        int ret = hash_table_insert(ht_p, &hk, kvtbl[i].value);
        if (ret < 0) puts("hash_table_insert failed");
        else puts("hash_table_insert: success");
    }
    puts("");

    puts("- 8. lookup all keys all over");
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
        hash_table_remove(ht_p, &hsh);
        ht_slot_handle_reset(&hsh);
    }


    destroy_hash_table(ht_p);
    return 0;
}
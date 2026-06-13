#define _KVHT_INTRNL_IMPLM

#include <kvfile.h>
#include <stdlib.h>
#include "_kvarena.inl"

int kvarena_build_memimg_buf(
    KVArena        *kva_p,
    unsigned char **out_filebuf_pp,
    size_t         *out_filesize_p
) {
    _dbg_print("kvarena_build_memimg_buf@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf) goto failed_ret;
    if (!kva_p->entrycnt) goto failed_ret;
    if (!out_filebuf_pp || !out_filesize_p) goto failed_ret;

    _dbg_print("kvarena_build_memimg_buf@1.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);

    KVFile kvf = {0};
    _dbg_print("kvarena_build_memimg_buf@2.KVFile_Init");
    KVFile_Init(&kvf);
    _dbg_print("kvarena_build_memimg_buf@2.KVFile_CreateBuilderBuffer");
    int ret =
        KVFile_CreateBuilderBuffer(
            &kvf,
            kva_p->data_buf_len,
            kva_p->entrycnt,
            kva_p->align
        );
    if (ret < 0) goto failed;
    _dbg_print("kvarena_build_memimg_buf@3.KVFileBuilder_WriteFileHeader");
    if (!KVFileBuilder_WriteFileHeader(&kvf)) goto failed;
    _dbg_print("kvarena_build_memimg_buf@4.KVFileBuilder_WriteEntryTable");
    if (!KVFileBuilder_WriteEntryTable(&kvf, kva_p->entrytbl)) goto failed;
    _dbg_print("kvarena_build_memimg_buf@5.KVFileBuilder_WriteDataSection");
    if (!KVFileBuilder_WriteDataSection(&kvf, kva_p->data_buf)) goto failed;
    _dbg_print("kvarena_build_memimg_buf@6.KVFileBuilder_WriteFileFooter");
    if (!KVFileBuilder_WriteFileFooter(&kvf)) goto failed;

    assert(kvf.buf_base);
    assert(kvf.buf_end);
    _dbg_print("kvarena_build_memimg_buf@7.kvfbuf_len");
    ptrdiff_t kvfbuf_len = kvf.buf_end - kvf.buf_base;
    assert(kvfbuf_len > 0);

    _dbg_print("kvarena_build_memimg_buf@8.move_out(kvf.buf_base)");
    *out_filebuf_pp = kvf.buf_base;
    *out_filesize_p = (size_t)kvfbuf_len;
    kvf.buf_base = NULL;
    kvf.buf_end = NULL;

    _dbg_print("kvarena_build_memimg_buf@9.KVFile_DestroyBuilderBuffer");
    KVFile_DestroyBuilderBuffer(&kvf);
    _dbg_print("kvarena_build_memimg_buf@10.KVFile_Fini");
    KVFile_Fini(&kvf);

    _dbg_print("kvarena_build_memimg_buf@0.ret\n");
    return 0;
failed:
    _dbg_print("kvarena_build_memimg_buf@-1.failed\n");
    KVFile_DestroyBuilderBuffer(&kvf);
    KVFile_Fini(&kvf);
failed_ret:
    _dbg_print("kvarena_build_memimg_buf@-1.failed_ret\n");
    return -1;
}

void kvarena_destroy_memimg_buf(
    KVArena        *kva_p,
    unsigned char **filebuf_pp
) {
    _dbg_print("kvarena_destroy_memimg_buf@0");
    (void)kva_p;
    if (!filebuf_pp) goto scope_end;
    _dbg_print("kvarena_destroy_memimg_buf@1.free");
    if (*filebuf_pp) {
        free(*filebuf_pp);
        *filebuf_pp = NULL;
    }
    _dbg_print("kvarena_destroy_memimg_buf@0.ret");
scope_end:
    return;
}

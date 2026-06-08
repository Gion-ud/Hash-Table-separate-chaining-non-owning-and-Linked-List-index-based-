/*kvimg.c; to be finished; kvarena && kvfile */
#include <kvfile.h>
#include "_kvarena.inl"

int kvarena_build_memimg_buf(
    KVArena        *kva_p,
    unsigned char **out_filebuf_pp,
    size_t         *out_filesize_p
) {
    _dbg_print("kvarena_build_memimg@0.validation");
    if (!kva_p || !kva_p->entrytbl || !kva_p->data_buf) goto failed_ret;
    if (!out_filebuf_pp || out_filesize_p) goto failed_ret;

    _dbg_print("kvarena_build_memimg@1.assert_intrnl_state");
    _kva_assert_intrnl_state(kva_p);



    return 0;
failed_ret:
    return -1;
}
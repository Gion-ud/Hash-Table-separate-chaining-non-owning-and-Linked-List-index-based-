#pragma once

#define _MEM_STREAM_ALLOW_HEAP_ALLOC

#include <stddef.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

typedef struct _mem_stream {
    unsigned char  *buf;
    unsigned long   rpos;
    unsigned long   wpos;
    unsigned long   end;
    unsigned long   size;
} mem_stream_t;

extern int mem_stream_open_mem(
    mem_stream_t   *stream_p,
    unsigned char  *buf,
    size_t          buf_size
);
extern int mem_stream_close_mem(
    mem_stream_t   *stream_p
);
extern void mem_stream_reset(
    mem_stream_t   *stream_p
);
extern long mem_stream_read(
    mem_stream_t   *stream_p,
    void           *out_buf,
    size_t          out_buf_len
);
extern long mem_stream_write(
    mem_stream_t   *stream_p,
    const void     *buf,
    size_t          buf_len
);
extern long mem_stream_lseek_rpos(
    mem_stream_t   *stream_p,
    long            off,
    int             origin
);
extern long mem_stream_lseek_wpos(
    mem_stream_t   *stream_p,
    long            off,
    int             origin
);
extern int mem_stream_truncate(
    mem_stream_t   *stream_p,
    unsigned long   length
);
extern void *mem_stream_data(const mem_stream_t *stream_p);
extern size_t *mem_stream_length(const mem_stream_t *stream_p);
extern size_t *mem_stream_size(const mem_stream_t *stream_p);

#ifdef _MEM_STREAM_ALLOW_HEAP_ALLOC
extern mem_stream_t *create_mem_stream(
    size_t  mem_size
);
extern void destroy_mem_stream(
    mem_stream_t   *sp
);
extern int mem_stream_resize(
    mem_stream_t   *sp,
    size_t          new_size
)
#endif /* _MEM_STREAM_ALLOW_HEAP_ALLOC */

//#define _MEM_STREAM_IMPLM /* debug */

#ifdef _MEM_STREAM_IMPLM
#include <string.h>

int mem_stream_open_mem(
    mem_stream_t   *stream_p,
    unsigned char  *buf,
    size_t          buf_size
) {
    if (!stream_p || !buf || !buf_size) return -1;
    memset(buf, 0, buf_size);
    stream_p->buf   = buf;
    stream_p->rpos  = 0;
    stream_p->wpos  = 0;
    stream_p->end   = 0;
    stream_p->size  = buf_size;
    return 0;
}

int mem_stream_close_mem(
    mem_stream_t   *stream_p
) {
    if (!stream_p) return -1;
    stream_p->buf   = NULL;
    stream_p->rpos  = 0;
    stream_p->wpos  = 0;
    stream_p->end   = 0;
    stream_p->size  = 0;
    return 0;
}

void mem_stream_reset(
    mem_stream_t   *stream_p
) {
    if (!stream_p) return;
    stream_p->rpos  = 0;
    stream_p->wpos  = 0;
    stream_p->end   = 0;
}

long mem_stream_write(
    mem_stream_t   *stream_p,
    const void     *buf,
    size_t          buf_len
) {
    if (!stream_p || !stream_p->buf || !buf || buf_len) goto failed;
    long rem = (long)stream_p->size - (long)stream_p->wpos;
    if (rem < 0) goto failed;
    if ((size_t)rem < buf_len) buf_len = (size_t)rem;
    memcpy(
        stream_p->buf + stream_p->wpos,
        buf,
        buf_len
    );
    stream_p->wpos += buf_len;
    if (stream_p->end < stream_p->wpos) stream_p->end = stream_p->wpos;

    return (long)buf_len;
failed:
    return -1L;
}

long mem_stream_read(
    mem_stream_t   *stream_p,
    void           *out_buf,
    size_t          out_buf_len
) {
    if (!stream_p || !stream_p->buf || !out_buf || out_buf_len) goto failed;
    long rem = (long)stream_p->end - (long)stream_p->rpos;
    if (rem < 0) goto failed;
    if ((size_t)rem < out_buf_len) out_buf_len = (size_t)rem;
    memcpy(
        out_buf,
        stream_p->buf + stream_p->rpos,
        out_buf_len
    );
    stream_p->rpos += out_buf_len;

    return (long)out_buf_len;
failed:
    return -1L;
}

long mem_stream_lseek_rpos(
    mem_stream_t   *stream_p,
    long            off,
    int             origin
) {
    if (!stream_p || !stream_p->buf) goto failed;
    long _rpos = 0;
    switch (origin) {
        case SEEK_SET:
            _rpos = off;
            break;
        case SEEK_CUR:
            _rpos = (long)stream_p->rpos + off;
            break;
        case SEEK_END:
            _rpos = (long)stream_p->end + off;
            break;
        default:
            goto failed;
    }
    if (_rpos > (long)stream_p->end) _rpos = (long)stream_p->end;
    stream_p->rpos = (unsigned long)_rpos;

    return stream_p->rpos;
failed:
    return -1L;
}

long mem_stream_lseek_wpos(
    mem_stream_t   *stream_p,
    long            off,
    int             origin
) {
    if (!stream_p || !stream_p->buf) goto failed;
    long _wpos = 0;
    switch (origin) {
        case SEEK_SET:
            _wpos = off;
            break;
        case SEEK_CUR:
            _wpos = (long)stream_p->wpos + off;
            break;
        case SEEK_END:
            _wpos = (long)stream_p->end + off;
            break;
        default:
            goto failed;
    }
    if (_wpos > (long)stream_p->size) _wpos = (long)stream_p->size;
    stream_p->wpos = (unsigned long)_wpos;

    return stream_p->wpos;
failed:
    return -1L;
}

int mem_stream_truncate(
    mem_stream_t   *stream_p,
    unsigned long   length
) {
    if (!stream_p || !stream_p->buf) goto failed;
    if (length > (long)stream_p->size) goto failed;
    stream_p->end = (unsigned long)length;
    return 0;
failed:
    return -1;
}

void *mem_stream_data(const mem_stream_t *stream_p) {
    return (!stream_p || !stream_p->buf) ? NULL : stream_p->buf;
}
size_t *mem_stream_length(const mem_stream_t *stream_p) {
    return (!stream_p || !stream_p->buf) ? 0 : stream_p->end;
}
size_t *mem_stream_size(const mem_stream_t *stream_p) {
    return (!stream_p || !stream_p->buf) ? 0 : stream_p->size;
}

#ifdef _MEM_STREAM_ALLOW_HEAP_ALLOC
#include <stdlib.h>
mem_stream_t *create_mem_stream(
    size_t  mem_size
) {
    mem_stream_t *sp = (mem_stream_t*)malloc(sizeof(mem_stream_t));
    if (!sp) goto failed_ret;
    sp->buf = (unsigned char*)malloc(mem_size);
    if (!sp->buf) {
        free(sp);
        goto failed_ret;
    }
    sp->rpos    = 0;
    sp->wpos    = 0;
    sp->end     = 0;
    sp->size    = mem_size;

    return sp;
failed_ret:
    return -1;
}

void destroy_mem_stream(
    mem_stream_t   *sp
) {
    if (!sp) return;
    if (sp->buf) free(sp->buf);
    free(sp);
}
#endif /* _MEM_STREAM_ALLOW_HEAP_ALLOC */

int mem_stream_resize(
    mem_stream_t   *sp,
    size_t          new_size
) {
    if (!sp || !sp->buf || !new_size) goto failed_ret;
    unsigned char *new_buf = (unsigned char*)realloc(sp->buf, new_size);
    if (!new_buf) goto failed_ret;
    sp->buf = new_buf;
    sp->size = new_size;
    return 0;
failed_ret:
    return -1;
}

#endif /* _MEM_STREAM_IMPLM */

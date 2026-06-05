#pragma once

#ifdef _DEBUG
#include <stdio.h>
#define _dbg_print(...) \
    do { \
        fprintf(stderr, __VA_ARGS__);putc('\n', stderr);fflush(stderr); \
    } while(0)
#else

#define _dbg_print(...)

#endif
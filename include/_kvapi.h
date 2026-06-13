#pragma once

#ifdef _BUILD_LIBKV_SHARED
#define LIBKV_API __attribute__((visibility("default"))) extern
#else
#define LIBKV_API extern
#endif
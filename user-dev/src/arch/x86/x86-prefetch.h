#ifndef _SRC__ARCH__X86__X86_PREFETCH_H_
#define _SRC__ARCH__X86__X86_PREFETCH_H_

#include "util/attrs.h"

#include <x86intrin.h>

static ALWAYS_INLINE
NONNULL(1) void ll_prefetchw(void * ptr) {
    _mm_prefetch(ptr, _MM_HINT_ET1);
}

static ALWAYS_INLINE
NONNULL(1) void ll_prefetch_L1(void * ptr) {
    _mm_prefetch(ptr, _MM_HINT_T0);
}

static ALWAYS_INLINE
NONNULL(1) void ll_prefetch_L2(void * ptr) {
    _mm_prefetch(ptr, _MM_HINT_T1);
}

static ALWAYS_INLINE
NONNULL(1) void ll_prefetch_L3(void * ptr) {
    _mm_prefetch(ptr, _MM_HINT_T2);
}

#endif

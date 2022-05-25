#ifndef _SRC__UTIL__MEMORY_UTIL_H_
#define _SRC__UTIL__MEMORY_UTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <wchar.h>

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"

#define PAGE_SIZE 4096

#define safe_calloc(n, sz) I_safe_calloc(n, sz, ERR_ARGS)
#define safe_zalloc(sz)    I_safe_calloc(sz, 1, ERR_ARGS)

#define safe_aligned_alloc(alignment, sz)                              \
    I_safe_aligned_alloc(alignment, sz, ERR_ARGS)

#define safe_realloc(p, sz) I_safe_realloc(p, sz, ERR_ARGS)
#define safe_srealloc(p, sz_old, sz_new)                               \
    I_safe_srealloc(p, sz_old, sz_new, ERR_ARGS)
#define safe_malloc(sz) I_safe_malloc(sz, ERR_ARGS)
#define safe_mmap(addr, sz, prot_flags, mmap_flags, fd, offset)        \
    I_safe_mmap(addr, sz, prot_flags, mmap_flags, fd, offset,          \
                __FILENAME__, __func__, __LINE__)
#define safe_mmap_alloc(sz)                                            \
    safe_mmap(NULL, sz, PROT_READ | PROT_WRITE,                        \
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#define safe_munmap(addr, sz) I_safe_munmap(addr, sz, ERR_ARGS)
#define safe_mprotect(addr, sz, prot_flags)                            \
    I_safe_mprotect(addr, sz, prot_flags, ERR_ARGS)

#define safe_free(addr)      I_safe_free(addr)
#define safe_sfree(addr, sz) I_safe_sfree(addr, sz)

#define is_valid_addr(addr)                                            \
    (CAST(uint64_t, addr) > CAST(uint64_t, 65536) &&                   \
     (!(CAST(uint64_t, addr) >> 48)))

static MALLOC_FUNC
NONNULL(3, 4) void * I_safe_calloc(uint64_t n,
                                   uint64_t sz,
                                   char const * restrict fn,
                                   char const * restrict func,
                                   uint32_t ln) {
    void * p = calloc_c(n, sz);
    if (UNLIKELY(p == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return p;
}

static MALLOC_FUNC
NONNULL(3, 4) void * I_safe_aligned_alloc(uint64_t alignment,
                                          uint64_t sz,
                                          char const * restrict fn,
                                          char const * restrict func,
                                          uint32_t ln) {
    void * p = aligned_alloc_c(alignment, sz);
    if (UNLIKELY(p == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return p;
}

static MALLOC_FUNC
NONNULL(2, 3) void * I_safe_malloc(uint64_t sz,
                                   char const * restrict fn,
                                   char const * restrict func,
                                   uint32_t ln) {
    void * p = malloc_c(sz);
    if (UNLIKELY(p == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return p;
}

static MALLOC_FUNC
NONNULL(1, 3, 4) void * I_safe_realloc(void * restrict p,
                                       uint64_t sz,
                                       char const * restrict fn,
                                       char const * restrict func,
                                       uint32_t ln) {
    void * newp = realloc_c(p, sz);
    if (UNLIKELY(p == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return newp;
}


static MALLOC_FUNC
NONNULL(1, 4, 5) void * I_safe_srealloc(void * restrict p,
                                        uint64_t sz_old,
                                        uint64_t sz_new,
                                        char const * restrict fn,
                                        char const * restrict func,
                                        uint32_t ln) {
    void * newp = srealloc_c(p, sz_old, sz_new);
    if (UNLIKELY(p == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return newp;
}

static void
I_safe_free(void * addr) {
    if (LIKELY(addr != NULL)) {
        free_c(addr);
    }
}


static void
I_safe_sfree(void * addr, uint64_t sz) {
    if (LIKELY(addr != NULL)) {
        sfree_c(addr, sz);
    }
}


MALLOC_FUNC NONNULL(7, 8) void * I_safe_mmap(void * restrict addr,
                                             uint64_t sz,
                                             int32_t  prot_flags,
                                             int32_t  mmap_flags,
                                             int32_t  fd,
                                             int32_t  offset,
                                             char const * restrict fn,
                                             char const * restrict func,
                                             uint32_t ln);

NONNULL(1, 3, 4)
void I_safe_munmap(void * restrict addr,
                   uint64_t sz,
                   char const * restrict fn,
                   char const * restrict func,
                   uint32_t ln);

NONNULL(1, 4, 5)
void I_safe_mprotect(void * restrict addr,
                     uint64_t sz,
                     int32_t  prot_flags,
                     char const * restrict fn,
                     char const * restrict func,
                     uint32_t ln);

#endif

#include "util/memory-util.h"
#include "util/common.h"
#include "util/error-util.h"

void *
I_safe_mmap(void * restrict addr,
            uint64_t sz,
            int32_t  prot_flags,
            int32_t  mmap_flags,
            int32_t  fd,
            int32_t  offset,
            char const * restrict fn,
            char const * restrict func,
            uint32_t ln) {
    void * p = mmap(addr, sz, prot_flags, mmap_flags, fd, offset);
    if (UNLIKELY(!is_valid_addr(p))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return p;
}

void
I_safe_munmap(void * restrict addr,
              uint64_t sz,
              char const * restrict fn,
              char const * restrict func,
              uint32_t ln) {
    if (UNLIKELY(munmap(addr, sz))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

void
I_safe_mprotect(void * restrict addr,
                uint64_t sz,
                int32_t  prot_flags,
                char const * restrict fn,
                char const * restrict func,
                uint32_t ln) {
    if (UNLIKELY(mprotect(addr, sz, prot_flags))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

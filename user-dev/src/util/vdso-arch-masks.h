#ifndef _SRC__UTIL__VDSO_ARCH_MASKS_H_
#define _SRC__UTIL__VDSO_ARCH_MASKS_H_

#include <stdint.h>

#include "util/vdso-func-enum.h"
#include "util/attrs.h"

static ALWAYS_INLINE CONST_FUNC uint32_t
get_x86_vdso_expec_mask() {
    return (1UL << vdso_clock_gettime_offset) |
           (1UL << vdso_gettimeofday_offset) | (1UL << vdso_getcpu_offset) |
           (1UL << vdso_time_offset);
}

static ALWAYS_INLINE CONST_FUNC uint32_t
get_vdso_expec_mask() {
#if __x86_64__
    return get_x86_vdso_expec_mask();
#else
    die("Unsupported arch!");
#endif
}

#endif

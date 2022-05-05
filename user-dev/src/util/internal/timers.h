#ifndef _SRC__UTIL__INTERNAL__TIMERS_H_
#define _SRC__UTIL__INTERNAL__TIMERS_H_

#include <time.h>

#include "util/attrs.h"
#include "util/types.h"

#include "arch/ll-syscall.h"

static ALWAYS_INLINE
NONNULL(2) void direct_clock_gettime(clockid_t clk, struct timespec * ts) {
    ll_syscall_cc(SYS_clock_gettime, (clk, ts), /* No +m */, /* No m */,
                  ((struct timespec(*)[1])ts));
}

#endif

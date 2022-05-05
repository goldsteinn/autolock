#ifndef _SRC__UTIL__TIMERS_H_
#define _SRC__UTIL__TIMERS_H_

#include <time.h>

#include "util/attrs.h"
#include "util/types.h"


#include "arch/ll-timing.h"
#include "util/internal/timers.h"
#include "util/vdso-util.h"

static ALWAYS_INLINE
NONNULL(1) void direct_gettime(struct timespec * ts) {
    direct_clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts);
}

static ALWAYS_INLINE
NONNULL(1) void gettime(struct timespec * ts) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts);
}

static ALWAYS_INLINE
NONNULL(1) void vdso_gettime(struct timespec * ts) {
    vdso_clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts);
}


#endif

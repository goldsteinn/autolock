#ifndef _SRC__UTIL__VDSO_UTIL_H_
#define _SRC__UTIL__VDSO_UTIL_H_

#include <sched.h>
#include <sys/time.h>
#include <time.h>

#include "util/attrs.h"
#include "util/types.h"
#include "util/verbosity.h"

#include "util/vdso-arch-masks.h"
#include "util/vdso-func-enum.h"

typedef void (*I_vdso_placeholder_f)(void);

extern I_vdso_placeholder_f vdso_funcs[];


uint32_t safe_vdso_init();
void     safe_vdso_init_all();
uint32_t vdso_init();

static ALWAYS_INLINE CONST_FUNC int32_t
is_vdso_init_error(uint32_t ret) {
    return ret == -1U;
}

static ALWAYS_INLINE CONST_FUNC int32_t
is_vdso_full_init(uint32_t ret) {
    return ret == get_vdso_expec_mask();
}

static ALWAYS_INLINE PURE_FUNC
FUNC_T(clock_gettime) get_vdso_clock_gettime() {
    return CAST_TO_FUNC(clock_gettime, vdso_funcs[vdso_clock_gettime_offset]);
}

static ALWAYS_INLINE int32_t
NONNULL(2) vdso_clock_gettime(clockid_t clockid, struct timespec * ts) {
    return get_vdso_clock_gettime()(clockid, ts);
}

static int32_t
NONNULL(1) vdso_gettimeofday(struct timeval * restrict tv,
                             struct timezone * restrict tz) {
    return (CAST_TO_FUNC(gettimeofday,
                         vdso_funcs[vdso_gettimeofday_offset]))(tv, tz);
}

static int32_t
NONNULL(1) vdso_getcpu(uint32_t * restrict cpu, uint32_t * restrict node) {
    return CAST_TO_FUNC(getcpu, vdso_funcs[vdso_getcpu_offset])(cpu, node);
}

static time_t
NONNULL(1) vdso_time(time_t * tloc) {
    return CAST_TO_FUNC(time, vdso_funcs[vdso_time_offset])(tloc);
}


#endif

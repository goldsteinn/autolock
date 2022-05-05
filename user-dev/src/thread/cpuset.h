#ifndef _SRC__THREAD__CPUSET_H_
#define _SRC__THREAD__CPUSET_H_


#include <sched.h>
#include <stdint.h>

#include "util/inline-math.h"

#ifdef CPU_SET_OPTIMIZED

typedef uint64_t cpuset_t;

static void
NONNULL(2) cset_set(uint32_t cpu, cpuset_t * cset) {
    *cset |= (1UL << cpu);
}

static uint64_t
NONNULL(2) cset_isset(uint32_t cpu, cpuset_t const * cset) {
    return (*cset & (1UL << cpu));
}

static uint32_t
NONNULL(1) cset_count(cpuset_t const * cset) {
    return __builtin_popcountll(*cset);
}


#else

typedef cpu_set_t cpuset_t;

#define cset_set   CPU_SET
#define cset_isset CPU_ISSET
#define cset_count CPU_COUNT

#endif

static void
NONNULL(1) cset_zero(cpuset_t * cset) {
    __builtin_memset(cset, 0, sizeof(cpuset_t));
}


#endif

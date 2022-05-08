#ifndef _SRC__THREAD__CPUSET_H_
#define _SRC__THREAD__CPUSET_H_


#include <sched.h>
#include <stdint.h>

#include "util/cpu-info.h"
#include "util/inline-math.h"


static void
NONNULL(1) cset_zero(cpuset_t * cset) {
    __builtin_memset(cset, 0, sizeof(cpuset_t));
}

static void
NONNULL(1, 2) cset_copy(cpuset_t * lhs, cpuset_t const * rhs) {
    __builtin_memcpy(lhs, rhs, sizeof(*rhs));
}

static cpuset_t
new_cpuset_zero() {
    cpuset_t cset;
    cset_zero(&cset);
    return cset;
}


#ifdef I_CPU_SET_OPTIMIZED

static void
NONNULL(1) cset_all(cpuset_t * cset) {
    uint32_t nprocs = get_num_cpus();
    cpuset_t ones   = TYPE_MAX(cpuset_t);
    ones >>= (-nprocs) % sizeof_bits(cpuset_t);
    *cset = ones;
}

static void
NONNULL(2) cset_set(uint32_t cpu, cpuset_t * cset) {
    *cset |= (1UL << cpu);
}

static void
NONNULL(2) cset_clr(uint32_t cpu, cpuset_t * cset) {
    *cset &= (~(1UL << cpu));
}

static void
NONNULL(1) cset_clr_first(cpuset_t * cset) {
    cpuset_t tmp = *cset;
    tmp &= (tmp - 1);
    *cset = tmp;
}

static uint64_t
NONNULL(2) cset_isset(uint32_t cpu, cpuset_t const * cset) {
    return (*cset & (1UL << cpu));
}

static uint32_t
NONNULL(1) cset_count(cpuset_t const * cset) {
    return popcnt(*cset);
}

static uint32_t
NONNULL(1) cset_empty(cpuset_t const * cset) {
    return (*cset) == 0;
}

static int32_t
NONNULL(1) cset_first(cpuset_t const * cset) {
    return cset_empty(cset) ? -1 : ctz(*cset);
}

static void
NONNULL(1, 2) cset_copy_first(cpuset_t * lhs, cpuset_t const * rhs) {
    cpuset_t tmp = *rhs;
    tmp &= (-tmp);
    *lhs = tmp;
}

static uint32_t
NONNULL(1, 2) cset_equal(cpuset_t const * lhs, cpuset_t const * rhs) {
    return *lhs == *rhs;
}

static void
NONNULL(1, 2) cset_or(cpuset_t *       dst,
                      cpuset_t const * src0,
                      cpuset_t const * src1) {
    *dst = (*src0) | (*src1);
}

static void
NONNULL(1, 2) cset_or_eq(cpuset_t * dst, cpuset_t const * src) {
    *dst |= *src;
}

static uint32_t
NONNULL(1, 2) cset_test(cpuset_t const * lhs, cpuset_t const * rhs) {
    return (*lhs & *rhs) != 0;
}

static ALWAYS_INLINE CONST_FUNC cpu_set_t const *
NONNULL(1, 2)
    cset_copy_to_std(cpuset_t const * orig, cpu_set_t * other) {
    __builtin_memset(other, 0, sizeof(*other));
    __builtin_memcpy(other, orig, sizeof(*orig));
    return other;
}


#else


#define cset_set   CPU_SET
#define cset_isset CPU_ISSET
#define cset_count CPU_COUNT
#define cset_equal CPU_EQUAL
#define cset_or    CPU_OR
#define cset_clr   CPU_CLR
#define cset_and   CPU_AND

static void
NONNULL(1, 2) cset_or_eq(cpuset_t * dst, cpuset_t const * src) {
    cset_or(dst, dst, src);
}

static int32_t
NONNULL(1) cset_empty(cpuset_t const * cset) {
    cpuset_t zero_set;
    cset_zero(&zero_set);
    return cset_equal(&zero_set, cset);
}

static uint32_t
NONNULL(1, 2) cset_test(cpuset_t const * lhs, cpuset_t const * rhs) {
    cpuset_t tmp;
    cset_and(&tmp, lhs, rhs);
    return !cset_empty(&tmp);
}

static uint32_t
NONNULL(1) cset_first(cpuset_t const * cset) {
    int32_t i;
    int32_t ncpus = get_num_cpus();
    die_assert(!cset_empty(cset));
    for (i = 0; i < ncpus; ++i) {
        if (cset_isset(i, cset)) {
            return i;
        }
    }
    return -1;
}

static void
NONNULL(1, 2) cset_copy_first(cpuset_t * lhs, cpuset_t const * rhs) {
    int32_t cpu = cset_first(rhs);
    cset_zero(lhs);
    die_assert(cpu != (-1));
    cset_set(cpu, lhs);
}

static void
NONNULL(1) cset_clr_first(cpuset_t * cset) {
    uint32_t cpu = cset_first(cset);
    die_assert(cpu != -1U);
    cset_set(cpu, cset);
}


static void
NONNULL(1) cset_all(cpuset_t * cset) {
    uint32_t nprocs = get_num_cpus();
    uint32_t i;
    cset_zero(cset);
    __builtin_memset(cset, -1, nprocs / CHAR_BIT);
    for (i = ROUNDDOWN_P2(nprocs, CHAR_BIT); i != nprocs; ++i) {
        cset_set(i, cset);
    }
}

static ALWAYS_INLINE CONST_FUNC cpu_set_t const *
NONNULL(1, 2)
    cset_copy_to_std(cpuset_t const * orig, cpu_set_t * other) {
    (void)(other);
    return orig;
}

#endif


NONNULL(1, 3)
char * cset_to_str(char * buf, uint64_t buflen, cpuset_t const * cset);

#endif

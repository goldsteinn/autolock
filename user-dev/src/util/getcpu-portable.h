#ifndef _SRC__UTIL__GETCPU_PORTABLE_H_
#define _SRC__UTIL__GETCPU_PORTABLE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>

#include "arch/ll-syscall.h"
#include "util/attrs.h"

/* getcpu() got GLIBC support in 2.29. NB: This is buggy if you build on
 * a system with GLIBC >= 2.29 then run on a system with glibc < 2.29.
 */
#if __GLIBC_PREREQ(2, 29) && defined(_GNU_SOURCE)
static NONNULL(1) int getcpu_p(unsigned int * cpu,
                               unsigned int * node) {
    return getcpu(cpu, node);
}
#else
static NONNULL(1) int getcpu_p(unsigned int * cpu,
                               unsigned int * node) {
    return ll_syscall_cc(
        SYS_getcpu, (cpu, node), /* no +m */, /* no m */,
        (((unsigned int(*)[1])cpu), ((unsigned int(*)[1])node)));
}
#endif


#endif

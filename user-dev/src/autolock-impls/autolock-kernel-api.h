#ifndef _SRC__AUTOLOCK_IMPLS__AUTOLOCK_KERNEL_API_H_
#define _SRC__AUTOLOCK_IMPLS__AUTOLOCK_KERNEL_API_H_

#include "util/common.h"
#include "util/memory-util.h"

#include "arch/ll-syscall.h"
#include "autolock-impls/autolock-kernel-abi.h"
#include "autolock-impls/internal/autolock-common-returns.h"

/********************************************************************/
/* Syscall numbers. */
enum { _NR_AUTOLOCK_CREATE = 451, _NR_AUTOLOCK_RELEASE = 452 };

extern_C_start();
/* Ideally this is initialized at TLS startup. */
extern __thread struct kernel_autolock_abi * I_kernel_autolock;

/********************************************************************/
/* API for intializing / releasing a autolock registered with the
 * kernel. Both return non-zero on failure and zero on success. */


static int32_t autolock_init_kernel_state();
static int32_t autolock_release_kernel_state();

static void autolock_set_kernel_watch_mem(uint32_t * p);
static void autolock_set_kernel_watch_for(uint32_t v);
static void autolock_set_kernel_watch_neq(uint32_t v);


/********************************************************************/
/* Start API implementation. */
static int32_t
autolock_init_kernel_state() {
    int32_t fd;
    void *  p;

    if (LIKELY(I_kernel_autolock != NULL)) {
        return I_SUCCESS;
    }

    /* Get fd back for shared memory mapping. */
    fd = ll_syscall_cc(
        _NR_AUTOLOCK_CREATE, /* None */, /* None */, /* None */,
        /* None */);
    /* Return non-zero value on failure. */
    if (UNLIKELY(fd < 0)) {
        return fd;
    }

    /* Map a page (wasteful, looking into to putting this on the stack
     * similiar to rseq). */
    p = CAST(void *,
             ll_syscall_cc(CAST(uint64_t, SYS_mmap),
                           (0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_POPULATE, fd, 0),
                           /* None */, /* None */,
                           /* None */));
    /* Return non-zero value on failure. */
    if (UNLIKELY(p == MAP_FAILED)) {
        return CAST(int32_t,
                    CAST(uint32_t, CAST(uint64_t, MAP_FAILED)));
    }

    /* Success, set thread's autolock and return zero. */
    I_kernel_autolock = (struct kernel_autolock_abi *)p;

    return I_SUCCESS;
}

static int32_t
autolock_release_kernel_state() {
    /* Nop, automatically done on process end. */
    return I_SUCCESS;
}

static void
autolock_set_kernel_watch_mem(uint32_t * p) {
    I_kernel_autolock->watch_mem = p;
}
static void
autolock_set_kernel_watch_for(uint32_t v) {
    I_kernel_autolock->watch_for = v;
}
static void
autolock_set_kernel_watch_neq(uint32_t v) {
    I_kernel_autolock->watch_neq = v;
}
extern_C_end();
#endif

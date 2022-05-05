#ifndef _SRC__AUTLOCK_IMPLS__AUTOLOCK_COMMON_H_
#define _SRC__AUTLOCK_IMPLS__AUTOLOCK_COMMON_H_

#include "arch/ll-syscall.h"
#include "autolock-impls/autolock-abi.h"

/********************************************************************/
/* Syscall numbers. */
enum { _NR_AUTOLOCK_CREATE = 451, _NR_AUTOLOCK_RELEASE = 452 };

/* Ideally this is initialized at TLS startup. */
extern __thread struct kernel_autolock_abi * kernel_autolock;

/********************************************************************/
/* API for intializing / releasing a autolock registered with the
 * kernel. Both return non-zero on failure and zero on success. */


static int32_t autolock_init_kernel_state();
static int32_t autolock_release_kernel_state();


/********************************************************************/
/* Start API implementation. */
static int32_t
autolock_init_kernel_state() {
    int32_t fd;
    void *  p;

    PRINTFFL;
    if (LIKELY(kernel_autolock != NULL)) {
        return 0;
    }
    PRINTFFL;

    /* Get fd back for shared memory mapping. */
    fd = ll_syscall_cc(
        _NR_AUTOLOCK_CREATE, /* None */, /* None */, /* None */,
        /* None */);
    PRINTFFL;
    /* Return non-zero value on failure. */
    if (UNLIKELY(fd < 0)) {
        PRINTFFL;
        return fd;
    }

    PRINTFFL;
    /* Map a page (wasteful, looking into to putting this on the stack
     * similiar to rseq). */
    p = CAST(void *,
             ll_syscall_cc(CAST(uint64_t, SYS_mmap),
                           (0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_POPULATE, fd, 0),
                           /* None */, /* None */,
                           /* None */));
    PRINTFFL;
    /* Return non-zero value on failure. */
    if (UNLIKELY(p == MAP_FAILED)) {
        PRINTFFL;
        return CAST(int32_t,
                    CAST(uint32_t, CAST(uint64_t, MAP_FAILED)));
    }

    PRINTFFL;
    /* Success, set thread's autolock and return zero. */
    kernel_autolock = (struct kernel_autolock_abi *)p;

    return 0;
}

static int32_t
autolock_release_kernel_state() {
    /* Nop, automatically done on process end. */
    return 0;
}

#endif

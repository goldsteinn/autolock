#ifndef _SRC__AUTOLOCK_IMPLS__AUTOLOCK_KERNEL_API_H_
#define _SRC__AUTOLOCK_IMPLS__AUTOLOCK_KERNEL_API_H_

#include "util/common.h"
#include "util/memory-util.h"

#include "arch/ll-syscall.h"
#include "autolock-impls/sys/autolock-kernel-abi.h"
#include "autolock-impls/common/autolock-returns.h"

/********************************************************************/
/* Syscall numbers. */
enum { _NR_AUTOLOCK_CREATE = 451, _NR_AUTOLOCK_RELEASE = 452 };

extern_C_start();
/* Ideally this is initialized at TLS startup. */
extern __thread struct kernel_autolock_abi * I_kernel_autolock
    ATTR_TLS_INIT_EXEC;

/********************************************************************/
/* API for intializing / releasing a autolock registered with the
 * kernel.  */


/* Return NULL on failure and pointer to kernel_autolock_abi on success.
 */
static struct kernel_autolock_abi * autolock_init_kernel_state();

/* Return non-zero on failure and zero on success. */
static int32_t autolock_release_kernel_state();

static NONNULL(2) void autolock_set_kernel_watch_mem(
    uint32_t * restrict p,
    struct kernel_autolock_abi * restrict k_autolock_mem);
static NONNULL(2) void autolock_set_kernel_watch_for(
    uint32_t                     v,
    struct kernel_autolock_abi * k_autolock_mem);
static NONNULL(2) void autolock_set_kernel_watch_neq(
    uint32_t                     v,
    struct kernel_autolock_abi * k_autolock_mem);

static struct kernel_autolock_abi * autolock_get_kernel_mem();
static NONNULL(1) void autolock_set_kernel_mem(
    struct kernel_autolock_abi * k_autolock_mem);


/********************************************************************/
/* Start API implementation. */
static struct kernel_autolock_abi *
autolock_init_kernel_state() {
    int32_t                      fd;
    void *                       p;
    struct kernel_autolock_abi * k_autolock_mem =
        autolock_get_kernel_mem();

    if (LIKELY(k_autolock_mem != NULL)) {
        return k_autolock_mem;
    }

    /* Get fd back for shared memory mapping. */
    fd = ll_syscall_cc(
        _NR_AUTOLOCK_CREATE, /* None */, /* None */, /* None */,
        /* None */);
    /* Return non-zero value on failure. */
    if (UNLIKELY(fd < 0)) {
        return NULL;
    }

    /* Map a page (wasteful, looking into to putting this on the stack
     * similiar to rseq). */
    p = CAST(void *,
             ll_syscall_cc(CAST(uint64_t, SYS_mmap),
                           (0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_POPULATE, fd, 0),
                           /* None */, /* None */,
                           /* None */));

    if (UNLIKELY(p == MAP_FAILED)) {
        return NULL;
    }

    /* Success, set thread's autolock and return zero. */
    autolock_set_kernel_mem((struct kernel_autolock_abi *)p);
    return (struct kernel_autolock_abi *)p;
}

static int32_t
autolock_release_kernel_state() {
    /* Nop, automatically done on process end. */
    return I_SUCCESS;
}

static void
autolock_set_kernel_watch_mem(
    uint32_t * restrict p,
    struct kernel_autolock_abi * restrict k_autolock_mem) {
    k_autolock_mem->watch_mem = p;
}
static void
autolock_set_kernel_watch_for(
    uint32_t                     v,
    struct kernel_autolock_abi * k_autolock_mem) {
    k_autolock_mem->watch_for = v;
}
static void
autolock_set_kernel_watch_neq(
    uint32_t                     v,
    struct kernel_autolock_abi * k_autolock_mem) {
    k_autolock_mem->watch_neq = v;
}


static struct kernel_autolock_abi *
autolock_get_kernel_mem() {
    return I_kernel_autolock;
}

static void
autolock_set_kernel_mem(struct kernel_autolock_abi * k_autolock_mem) {
    I_kernel_autolock = k_autolock_mem;
}

extern_C_end();
#endif

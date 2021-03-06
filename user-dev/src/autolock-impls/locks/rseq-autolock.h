#ifndef _SRC__AUTOLOCK_IMPLS__RSEQ_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__RSEQ_AUTOLOCK_H_

/* Where the common user-level autolock features are implemented. This
 * includes the lock type, and init/destroy/unlock/trylock functions. */
#include "internal/autolock-common-user-api.h"
#include "thread/rseq/rseq.h"


/* For now don't support rseq for shared library interpose because TLS
 * access is 1) indirect and 2) maintaing the assembly for both
 * compatability with PTHREAD_MUTEX_INITIALIZER and optimal is not worth
 * it unless we are 100% sure this is the lock we want to present. */
#if (!defined I_HAS_RSEQ) ||                                           \
    (defined I_USE_FOR_SHARED_LIBRARY_INTERPOSE)
#define RSEQ_AUTOLOCK
#else
#define RSEQ_AUTOLOCK rseq_autolock
/* The user-level lock type and functions {init|destroy|trylock|unlock}
 * are all essentially unchanging so just use alias them to common
 * defintions. */
typedef I_user_autolock_t rseq_autolock_t;
static typedef_func(rseq_autolock_init, I_user_autolock_init);
static typedef_func(rseq_autolock_destroy, I_user_autolock_destroy);
static typedef_func(rseq_autolock_trylock, I_user_autolock_trylock);
static typedef_func(rseq_autolock_unlock, I_user_autolock_unlock);

static NONNULL(1) int32_t rseq_autolock_lock(rseq_autolock_t * lock) {
    /* Hot path before entering assembly (we path a bit just to setup
     * rseq section and its hard to provide a hot path in inline
     * assembly. */
    if (LIKELY(rseq_autolock_trylock(lock) == I_SUCCESS)) {
        return I_SUCCESS;
    }
    autolock_init_kernel_state();

    /* We only have an autolock + rseq impl for x86_64. */
#ifndef __x86_64__
#error "Unsupported!"
#endif

    /* These values are assumed by the assembly. */
    const_assert(I_UNLOCKED == 1);
    const_assert(I_LOCKED == 0);

    const_assert(I_SUCCESS == 0);
    const_assert(I_FAILURE == 1);
    asm volatile(
        RSEQ_INIT_NEW_CS(%%rax)

        /* Setup kernel autolock. We are merging the zero write for
           watch_for and watch_neq. */
        "movq   $1, %%fs:I_kernel_autolock@tpoff+8\n\t"

        /* Sets `eax` with low bits which is all we need. */
        "rdtsc\n\t"

        /* Put the backoff in the critical section. This allows us to
           instantly exit on re-sched. */
        "1:\n\t"
        /* Arm/re-arm lock. */
        "movq   %[lock_mem], %%fs:I_kernel_autolock@tpoff+0\n"

        /* Get next backoff iter. */
        "leal   1(%%rax, %%rax), %%eax\n\t"
        "movzbl %%al, %%edx\n\t"
        "7:\n\t"
        "pause\n\t"
        "decl   %%edx\n\t"
        /* If edx is zero we don't want to loop 4 billion
           times. `decl` can macrofuse with `jg` */
        "jg    7b\n\t"
        "2:\n\t"

        /* Test the lock. No need for this to be in the critical section. */
        "movl   (%[lock_mem]), %%edx\n\t"
        "testl  %%edx, %%edx\n\t"
        /* If lock_mem is zero (not I_UNLOCKED) then go to backoff
           loop. */
        "jz    1b\n\t"
        /* Disarm lock. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+0\n"

        /* 0 == I_LOCKED. */
        "xorl   %%edx, %%edx\n\t"

        /* Implied `lock` for x86_64 xchg. */
        "xchg   %%edx, (%[lock_mem])\n\t"
        "testl  %%edx, %%edx\n\t"
        /* Failed to acquire the lock. */
        "jz    1b\n\t"
        "8:\n\t"
        /* In the context of autolock we go to the abort handler when we
           get rescheduled. This means there is a high likelyhood of the
           lock being available. */
        RSEQ_START_ABORT_DEF()
        "movl   (%[lock_mem]), %%edx\n\t"
        "testl  %%edx, %%edx\n\t"
        /* If lock_mem is zero (not I_UNLOCKED) then go to backoff
           loop. */
        "jz    1b\n\t"

        /* Disarm autolock before potentially grabbing the lock. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+0\n"

        /* 0 == I_LOCKED. */
        "xorl   %%edx, %%edx\n\t"
        /* Implied `lock` for x86_64 xchg. */
        "xchg   %%edx, (%[lock_mem])\n\t"

        "testl  %%edx, %%edx\n\t"
        /* Acquire the lock. */
        "jnz    8b\n\t"
        "jmp    1b\n\t"

        RSEQ_END_ABORT_DEF()
        : "+m"(*((uint32_t(*)[1])(&(lock->mem)))),
          "=m"(*((uint32_t * (*)[1])(&(I_kernel_autolock->watch_mem)))),
          "=m"(*((uint32_t(*)[1])(&(I_kernel_autolock->watch_for)))),
          "=m"(*((uint32_t(*)[1])(&(I_kernel_autolock->watch_neq))))
        : [lock_mem] "r"(lock)
        : "eax", "edx", "cc"
      );
    return I_SUCCESS;
}

#endif
#endif

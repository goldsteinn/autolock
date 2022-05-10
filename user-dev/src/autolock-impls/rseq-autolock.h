#ifndef _SRC__AUTOLOCK_IMPLS__RSEQ_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__RSEQ_AUTOLOCK_H_

/* Where the common user-level autolock features are implemented. This
 * includes the lock type, and init/destroy/unlock/trylock functions. */
#include "internal/autolock-common-user-api.h"
#include "thread/rseq/rseq.h"

/* The user-level lock type and functions {init|destroy|trylock|unlock}
 * are all essentially unchanging so just use alias them to common
 * defintions. */
typedef I_user_autolock_t rseq_autolock_t;
static typedef_func(rseq_autolock_init, I_user_autolock_init);
static typedef_func(rseq_autolock_destroy, I_user_autolock_destroy);
static typedef_func(rseq_autolock_trylock, I_user_autolock_trylock);
static typedef_func(rseq_autolock_unlock, I_user_autolock_unlock);

static NONNULL(1) void rseq_autolock_lock(rseq_autolock_t * lock) {
    /* Hot path before entering assembly (we path a bit just to setup
     * rseq section and its hard to provide a hot path in inline
     * assembly. */
    if (LIKELY(rseq_autolock_trylock(lock) == I_UNLOCKED)) {
        return;
    }
    autolock_init_kernel_state();

    /* We only have an autolock + rseq impl for x86_64. */
#ifndef __x86_64__
#error "Unsupported!"
#endif

    /* These values are assumed by the assembly. */
    const_assert(I_UNLOCKED == 0);
    const_assert(I_LOCKED == 1);
    __asm__ volatile goto(
        RSEQ_INIT_NEW_CS(%%rax)

        /* Setup kernel autolock. We are merging the zero write for
           watch_for and watch_neq. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+8\n\t"


        /* Sets `eax` with low bits which is all we need. */
        "rdtsc\n\t"

        /* Put the backoff in the critical section. This allows us to
           instantly exit on re-sched. */
        "1:\n\t"
        /* Rearm lock. */
        "movq   %[lock_mem], %%fs:I_kernel_autolock@tpoff+0\n"

        /* Get next backoff iter. */
        "leal   1(%%rax, %%rax), %%eax\n\t"
        /* Use edx as second temporary because its clobbered anyways
           by rdtsc. */
        "movzbl %%al, %%edx\n\t"
        "7:\n\t"
        "pause\n\t"
        "decl   %%edx\n\t"
        /* If edx is zero we don't want to loop 4 billion
           times. `decl` can macrofuse with `jg` */
        "jg    7b\n\t"
        "2:\n\t"

        /* Test the lock. No need for this to be in the critical section. */
        "movl   (%[lock_mem]), %%eax\n\t"
        "testl  %%eax, %%eax\n\t"
        /* If lock_mem is non-zero (not I_UNLOCKED) then go to backoff
           loop. */
        "jnz    1b\n\t"
        /* Disarm lock. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+0\n"
        /* Implied `lock` for x86_64 xchg. */
        "xchg   %%eax, (%[lock_mem])\n\t"
        "testl  %%eax, %%eax\n\t"
        /* Failed to acquire the lock. */
        "jnz    1b\n\t"

        /* In the context of autolock we go to the abort handler when we
           get rescheduled. This means there is a high likelyhood of the
           lock being available. */
        RSEQ_START_ABORT_DEF()
        "movl   (%[lock_mem]), %%eax\n\t"
        "testl  %%eax, %%eax\n\t"
        /* If lock_mem is non-zero (not I_UNLOCKED) then go to backoff
           loop. */
        "jnz    1b\n\t"

        /* NOAH_Todo: Verify that the interrupt path for scheduling a
                      task disarmed autolock. This will allow us to
                      optimize out this write (or use rseq w.o
                      interrupt support). */

        /* For now disarm autolock before potentially grabbing the lock. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+0\n"

        /* Disarm lock. */
        "movq   $0, %%fs:I_kernel_autolock@tpoff+0\n"
        /* Implied `lock` for x86_64 xchg. */
        "xchg   %%eax, (%[lock_mem])\n\t"

        "testl  %%eax, %%eax\n\t"
        /* Failed to acquire the lock. */
        "jnz    1b\n\t"

        /* We can't fall through here because of the stack protection. */
        "jmp %l[acquired]\n\t"
        RSEQ_END_ABORT_DEF()
        "\n\t"
        : "+m"(*(uint32_t(*)[1])(&(lock->mem))),
          "=m"(*(uint32_t * (*)[1])(&(I_kernel_autolock->watch_mem))),
          "=m"(*(uint32_t(*)[1])(&(I_kernel_autolock->watch_for))),
          "=m"(*(uint32_t(*)[1])(&(I_kernel_autolock->watch_neq)))
        : [lock_mem] "r"(lock)
        : "eax", "edx", "cc"
        : acquired);
acquired:
    return;
}


#endif

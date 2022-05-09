#ifndef _SRC__LOCKS__SIMPLE_AUTOLOCK_H_
#define _SRC__LOCKS__SIMPLE_AUTOLOCK_H_


#include "arch/ll-pause.h"
#include "autolock-impls/autolock-api.h"

/********************************************************************/
/* Lock structure. */
typedef struct simple_autolock {
    uint32_t
        mem; /* All we need is something to lock on. Must be u32. */
} simple_autolock_t;

enum { I_UNLOCKED = 0, I_LOCKED = 1 };


/* All functions expect autolock_init() to have been called first. */
/********************************************************************/
/* API for trylock, lock, and unlock respectively. */
static NONNULL(1) void simple_autolock_init(simple_autolock_t * lock);
static NONNULL(1) void simple_autolock_destroy(
    simple_autolock_t * lock);

static NONNULL(1) int32_t
    simple_autolock_trylock(simple_autolock_t * lock);
static NONNULL(1) void simple_autolock_lock(simple_autolock_t * lock);
static NONNULL(1) void simple_autolock_unlock(simple_autolock_t * lock);


/********************************************************************/
/* Internal functions. */
static NONNULL(1) int32_t
    I_simple_autolock_trylock_maybe_sched(simple_autolock_t * lock);


/********************************************************************/
/* Start API Implementation. */

static NONNULL(1) void simple_autolock_init(simple_autolock_t * lock) {
    die_assert(autolock_init_kernel_state() == 0);
    lock->mem = I_UNLOCKED;
}

static NONNULL(1) void simple_autolock_destroy(
    simple_autolock_t * lock) {
    autolock_release_kernel_state();
    (void)(lock);
}


static NONNULL(1) int32_t
    simple_autolock_trylock(simple_autolock_t * lock) {
    int32_t ret;
    /* Only do atomic write if we have a chance. */
    if ((ret = __atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) ==
        I_UNLOCKED) {
        if ((ret = __atomic_exchange_n(&(lock->mem), I_LOCKED,
                                       __ATOMIC_RELAXED)) ==
            I_UNLOCKED) {
            return I_UNLOCKED;
        }
    }
    GURANTEED(ret != I_UNLOCKED);
    return ret;
}


static NONNULL(1) void simple_autolock_lock(simple_autolock_t * lock) {
    /* Uncontended just grab the lock. No intialization is needed. */
    if (simple_autolock_trylock(lock) == I_UNLOCKED) {
        return;
    }
    autolock_init_kernel_state();


    /* Initialize kernel for this simple_autolock. */
    kernel_autolock->watch_for = I_UNLOCKED; /* What for unlocked. */
    kernel_autolock->watch_neq = 0;          /* Schedule when equals. */
    for (;;) {
        /* I_simple_autolock_trylock_maybe_sched will manage enabling /
         * disabling autolock in a safe manner. */
        if (I_simple_autolock_trylock_maybe_sched(lock) == I_UNLOCKED) {
            /* watch_mem will be NULL. */
            return;
        }
        /* pause to avoid runaway speculation. Note backoff could be
         * added VERY efficiently if we paired this with rseq. */
        ll_pause();
    }
}

/* Just update state. That will allow potentially de-scheduled threads
 * to start again. */
static NONNULL(1) void simple_autolock_unlock(
    simple_autolock_t * lock) {
    __atomic_store_n(&(lock->mem), I_UNLOCKED, __ATOMIC_RELAXED);
}


/********************************************************************/
/* Start internl function implementation. */

/* Trylock for autolock after we are watching the lock memory. */
static NONNULL(1) int32_t
    I_simple_autolock_trylock_maybe_sched(simple_autolock_t * lock) {
    int32_t ret;
    /* Only do atomic write if we have a chance. */
    if ((ret = __atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) ==
        I_UNLOCKED) {
        /*
         * Enable scheduling by setting `watch_mem = NULL`. This
         * disables autolock for the kernel. This is necessary to
         * prevent the following deadlock:
         *
         * 1. Current thread has `watch_mem = &(lock->mem).
         * 2. Current thread wins the lock (lock->mem now points to
         *    I_LOCKED).
         * 3. Current thread gets interuppted.
         * 4. Kernel now doesn't schedule current thread because
         *    **watch_mem == I_LOCKED and we are waiting for
         *    **watch_mem == I_UNLOCKED.
         * 5. Thread that holds the lock is now never scheduled so lock
         *    is never unlocked so we hit a deadlock.
         */
        kernel_autolock->watch_mem = NULL;
        if ((ret = __atomic_exchange_n(&(lock->mem), I_LOCKED,
                                       __ATOMIC_RELAXED)) ==
            I_UNLOCKED) {
            /* We successfully acquired the lock. */
            return I_UNLOCKED;
        }
    }

    /* Fall through path, we didn't acquire the lock (it is locked by
     * another thread) so re-enable autolock by setting `watch_mem =
     * &(lock->mem)`. NB: kernel_lock is thread local. There is no
     * contention writing to it. */
    kernel_autolock->watch_mem = &(lock->mem);

    /* Just a hint to the compiler so if it inlines this function is can
     * merge this patch with loop-continue. */
    GURANTEED(ret != I_UNLOCKED);
    return ret;
}


#endif

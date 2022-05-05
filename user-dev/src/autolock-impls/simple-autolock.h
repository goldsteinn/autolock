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
    PRINTFFL;
    /* Only do atomic write if we have a chance. */
    if ((ret = __atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) ==
        I_UNLOCKED) {
        PRINTFFL;
        if ((ret = __atomic_exchange_n(&(lock->mem), I_LOCKED,
                                       __ATOMIC_RELAXED)) ==
            I_UNLOCKED) {
            PRINTFFL;
            return I_UNLOCKED;
        }
    }
    PRINTFFL;
    GURANTEED(ret != I_UNLOCKED);
    PRINTFFL;
    return ret;
}


static NONNULL(1) void simple_autolock_lock(simple_autolock_t * lock) {
    /* Uncontended just grab the lock. No intialization is needed. */
    PRINTFFL;
    if (simple_autolock_trylock(lock) == I_UNLOCKED) {
        PRINTFFL;
        return;
    }
    autolock_init_kernel_state();

    
    PRINTFFL;
    /* Initialize kernel for this simple_autolock. */
    kernel_autolock->watch_for = I_UNLOCKED; /* What for unlocked. */
    kernel_autolock->watch_neq = 0;          /* Schedule when equals. */
    PRINTFFL;
    for (;;) {
        PRINTFFL;
        /* I_simple_autolock_trylock_maybe_sched will manage setting
         * kernel_autolock->watch_mem. */
        if (I_simple_autolock_trylock_maybe_sched(lock) == I_UNLOCKED) {
            PRINTFFL;
            /* watch_mem will be NULL. */
            return;
        }
        PRINTFFL;
        /* pause to avoid runaway speculation. Note backoff could be
         * added VERY efficiently if we paired this with rseq. */
        ll_pause();
    }
}

/* Just update state. That will allow potentially de-scheduled threads
 * to start again. */
static NONNULL(1) void simple_autolock_unlock(
    simple_autolock_t * lock) {
    PRINTFFL;
    __atomic_store_n(&(lock->mem), I_UNLOCKED, __ATOMIC_RELAXED);
}


/********************************************************************/
/* Start internl function implementation. */

/* Trylock for autolock after we are watching the lock memory. */
static NONNULL(1) int32_t
    I_simple_autolock_trylock_maybe_sched(simple_autolock_t * lock) {
    PRINTFFL;
    int32_t ret;
    /* Only do atomic write if we have a chance. */
    if ((ret = __atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) ==
        I_UNLOCKED) {
        PRINTFFL;
        /* Enable scheduling. Otherwise we may deadlock if we get
         * de-scheduled right after acquiring. */
        kernel_autolock->watch_mem = NULL;
        PRINTFFL;
        if ((ret = __atomic_exchange_n(&(lock->mem), I_LOCKED,
                                       __ATOMIC_RELAXED)) ==
            I_UNLOCKED) {
            PRINTFFL;
            return I_UNLOCKED;
            PRINTFFL;
        }
    }
    PRINTFFL;
    kernel_autolock->watch_mem = &(lock->mem);
    PRINTFFL;
    GURANTEED(ret != I_UNLOCKED);
    PRINTFFL;
    return ret;
}


#endif

#ifndef _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_COMMON_USER_API_H_
#define _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_COMMON_USER_API_H_


/* Common functionality for all user level autolock implementations.
 * Never include this file directly! */

#include "arch/ll-pause.h"
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/sys/autolock-kernel-api.h"
#include "util/common.h"
/********************************************************************/


#ifdef I_USE_FOR_SHARED_LIBRARY_INTERPOSE
/* For compatibility with PTHREAD_MUTEX_INITIALIZER must be I_UNLOCKED =
 * 0, I_LOCKED = 1. This is not ideal (from efficiency point of view)
 * but is necessary for benchmarking. */
enum { I_UNLOCKED = 0, I_LOCKED = 1 };

#else

enum { I_UNLOCKED = 1, I_LOCKED = 0 };

#endif


/* Lock structure. */
typedef struct I_user_autolock {
#ifdef I_WITH_FUTEX
    struct {
        union {
            uint32_t mem;
            uint8_t  padding0[8];
        } ALIGNED(8);
        union {
            uint32_t waiters; /* Waiters so unlock can know to futex
                                 unlock or not. */
            uint8_t padding1[8];
        } ALIGNED(8);
    } ALIGNED(16);
#else
    uint32_t
        mem; /* All we need is something to lock on. Must be u32. */
#endif
} I_user_autolock_t;

/* For aliasing functions easily. */
extern_C_start();

/* All functions expect autolock_init() to have been called first. */
/********************************************************************/
/* API for trylock, lock, and unlock respectively.  All functions return
 * zero on success and non-zero on error. */
static NONNULL(1) int32_t
    I_user_autolock_init(I_user_autolock_t * lock);
static NONNULL(1) int32_t
    I_user_autolock_destroy(I_user_autolock_t * lock);

/* Returns zero if lock was successfully acquired and non-zero
 * otherwise. */
static NONNULL(1) int32_t
    I_user_autolock_trylock(I_user_autolock_t * lock);
static NONNULL(1) int32_t
    I_user_autolock_unlock(I_user_autolock_t * lock);

/* The _lock() function varies. The rest are essentially fixed. */


/********************************************************************/
/* Internal functions. */
static NONNULL(1, 2) int32_t
    I_internal_user_autolock_trylock_maybe_sched(
        I_user_autolock_t * restrict lock,
        struct kernel_autolock_abi * restrict k_autolock_mem);


/********************************************************************/
/* Start API Implementation. */
static int32_t
I_user_autolock_init(I_user_autolock_t * lock) {
    lock->mem = I_UNLOCKED;
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    /* Force compiler to implement the returns with a branch as opposed
     * to cmov. */
    asm volatile("" : : :);
    return I_SUCCESS;
}

static int32_t
I_user_autolock_destroy(I_user_autolock_t * lock) {
    (void)(lock);
    return autolock_release_kernel_state();
}


static int32_t
I_user_autolock_trylock(I_user_autolock_t * lock) {
    /* Only do atomic write if we have a chance. */
    if ((__atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) != I_LOCKED) {
        if ((__atomic_exchange_n(&(lock->mem), I_LOCKED,
                                 __ATOMIC_RELAXED)) != I_LOCKED) {
            return I_SUCCESS;
        }
    }
    return I_FAILURE;
}


/* Just update state. That will allow potentially de-scheduled threads
 * to start again. */
static int32_t
I_user_autolock_unlock(I_user_autolock_t * lock) {
    __atomic_store_n(&(lock->mem), I_UNLOCKED, __ATOMIC_RELAXED);
    return I_SUCCESS;
}


/********************************************************************/
/* Start internl function implementation. */

/* Trylock for autolock after we are watching the lock memory. */
static int32_t
I_internal_user_autolock_trylock_maybe_sched(
    I_user_autolock_t * restrict lock,
    struct kernel_autolock_abi * restrict k_autolock_mem) {
    /* Only do atomic write if we have a chance. */
    if ((__atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) != I_LOCKED) {
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
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
        if ((__atomic_exchange_n(&(lock->mem), I_LOCKED,
                                 __ATOMIC_RELAXED)) != I_LOCKED) {
            /* We successfully acquired the lock. */
            return I_SUCCESS;
        }
    }

    /* Fall through path, we didn't acquire the lock (it is locked by
     * another thread) so re-enable autolock by setting `watch_mem =
     * &(lock->mem)`. NB: kernel_lock is thread local. There is no
     * contention writing to it. */
    autolock_set_kernel_watch_mem(&(lock->mem), k_autolock_mem);

    return I_FAILURE;
}


extern_C_end();
#endif

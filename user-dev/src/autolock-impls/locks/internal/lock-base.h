/* Base lock implementation for comparing autolock vs other. */

#ifndef I_WITH_AUTOLOCK
#error "Must define I_WITH_AUTOLOCK before including common api.h"
#endif
#ifndef I_WITH_BACKOFF
#error "Must define I_WITH_BACKOFF before include lock-base.h"
#endif
#ifndef I_WITH_YIELD
#error "Must define I_WITH_YIELD before include lock-base.h"
#endif
#ifndef I_WITH_FUTEX
#error "Must define I_WITH_FUTEX before include lock-base.h"
#endif

#undef LOCK_BASE_MAKE_NAME
#define LOCK_BASE_MAKE_NAME(name)                                      \
    CAT(name, I_WITH_AUTOLOCK, I_WITH_YIELD, I_WITH_BACKOFF,           \
        I_WITH_FUTEX)

#include "arch/ll-pause.h"
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/sys/autolock-kernel-api.h"
#include "util/common.h"
/********************************************************************/


#ifdef I_USE_FOR_SHARED_LIBRARY_INTERPOSE
/* For compatibility with PTHREAD_MUTEX_INITIALIZER must be I_UNLOCKED =
 * 0, I_LOCKED = 1. This is not ideal (from efficiency point of view)
 * but is necessary for benchmarking. */
enum {
    LOCK_BASE_MAKE_NAME(I_UNLOCKED) = 0,
    LOCK_BASE_MAKE_NAME(I_LOCKED)   = 1
};

#else

enum {
    LOCK_BASE_MAKE_NAME(I_UNLOCKED) = 1,
    LOCK_BASE_MAKE_NAME(I_LOCKED)   = 0
};

#endif


/* Lock structure. */
typedef struct LOCK_BASE_MAKE_NAME(I_user_lock_base) {
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
} LOCK_BASE_MAKE_NAME(I_user_lock_base_t);

/* For aliasing functions easily. */
extern_C_start();

/* All functions expect autolock_init() to have been called first. */
/********************************************************************/
/* API for trylock, lock, and unlock respectively.  All functions return
 * zero on success and non-zero on error. */
static NONNULL(1) int32_t LOCK_BASE_MAKE_NAME(I_user_lock_base_init)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock);
static NONNULL(1) int32_t LOCK_BASE_MAKE_NAME(I_user_lock_base_destroy)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock);

/* Returns zero if lock was successfully acquired and non-zero
 * otherwise. */
static NONNULL(1) int32_t LOCK_BASE_MAKE_NAME(I_user_lock_base_trylock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock);
static NONNULL(1) int32_t LOCK_BASE_MAKE_NAME(I_user_lock_base_unlock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock);

static NONNULL(1) int32_t LOCK_BASE_MAKE_NAME(I_user_lock_base_lock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock);


/********************************************************************/
/* Internal functions. */
static NONNULL(1) int32_t
    LOCK_BASE_MAKE_NAME(I_internal_user_lock_base_trylock_maybe_sched)(
        LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * restrict lock,
        struct kernel_autolock_abi * restrict k_autolock_mem);


/********************************************************************/
/* Start API Implementation. */
static int32_t
LOCK_BASE_MAKE_NAME(I_user_lock_base_init)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock) {
    lock->mem = LOCK_BASE_MAKE_NAME(I_UNLOCKED);
#if I_WITH_FUTEX
    lock->waiters = 0;
#endif
#if I_WITH_AUTOLOCK
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    /* Force compiler to implement the returns with a branch as opposed
     * to cmov. */
    asm volatile("" : : :);
#endif
    return I_SUCCESS;
}

static int32_t
LOCK_BASE_MAKE_NAME(I_user_lock_base_destroy)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock) {
    (void)(lock);
#if I_WITH_AUTOLOCK
    return autolock_release_kernel_state();
#else
    return I_SUCCESS;
#endif
}


static int32_t
LOCK_BASE_MAKE_NAME(I_user_lock_base_trylock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock) {
    /* Only do atomic write if we have a chance. */
    if ((__atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) !=
        LOCK_BASE_MAKE_NAME(I_LOCKED)) {
        if ((__atomic_exchange_n(
                &(lock->mem), LOCK_BASE_MAKE_NAME(I_LOCKED),
                __ATOMIC_RELAXED)) != LOCK_BASE_MAKE_NAME(I_LOCKED)) {
            return I_SUCCESS;
        }
    }
    return I_FAILURE;
}


/* Just update state. That will allow potentially de-scheduled threads
 * to start again. */
static int32_t
LOCK_BASE_MAKE_NAME(I_user_lock_base_unlock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock) {
    __atomic_store_n(&(lock->mem), LOCK_BASE_MAKE_NAME(I_UNLOCKED),
                     __ATOMIC_RELAXED);
#if I_WITH_FUTEX
    if (UNLIKELY(__atomic_load_n(&(lock->waiters), __ATOMIC_RELAXED))) {
        /* Futex error here could lead to deadlock so don't
         * silently ignore. */
        if (UNLIKELY(futex_wake(&(lock->mem), 1 /* wake one. */))) {
            return I_FAILURE;
        }
    }

#endif
    return I_SUCCESS;
}


/********************************************************************/
/* Start internl function implementation. */

/* Trylock for autolock after we are watching the lock memory. */
static int32_t
LOCK_BASE_MAKE_NAME(I_internal_user_lock_base_trylock_maybe_sched)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * restrict lock,
    struct kernel_autolock_abi * restrict k_autolock_mem) {
    /* Unused if no autolock.  */
    (void)(k_autolock_mem);
    /* Only do atomic write if we have a chance. */
    if ((__atomic_load_n(&(lock->mem), __ATOMIC_RELAXED)) !=
        LOCK_BASE_MAKE_NAME(I_LOCKED)) {
#if I_WITH_AUTOLOCK
        /*
         * Enable scheduling by setting `watch_mem = NULL`. This
         * disables autolock for the kernel. This is necessary to
         * prevent the following deadlock:
         *
         * 1. Current thread has `watch_mem = &(lock->mem).
         * 2. Current thread wins the lock (lock->mem now points to
         *    LOCK_BASE_MAKE_NAME(I_LOCKED)).
         * 3. Current thread gets interuppted.
         * 4. Kernel now doesn't schedule current thread because
         *    **watch_mem == LOCK_BASE_MAKE_NAME(I_LOCKED) and we are
         * waiting for
         *    **watch_mem == LOCK_BASE_MAKE_NAME(I_UNLOCKED).
         * 5. Thread that holds the lock is now never scheduled so lock
         *    is never unlocked so we hit a deadlock.
         */
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
        if ((__atomic_exchange_n(
                &(lock->mem), LOCK_BASE_MAKE_NAME(I_LOCKED),
                __ATOMIC_RELAXED)) != LOCK_BASE_MAKE_NAME(I_LOCKED)) {
            /* We successfully acquired the lock. */
            return I_SUCCESS;
        }
    }
#if I_WITH_AUTOLOCK
    /* Fall through path, we didn't acquire the lock (it is locked by
     * another thread) so re-enable autolock by setting `watch_mem =
     * &(lock->mem)`. NB: kernel_lock is thread local. There is no
     * contention writing to it. */
    autolock_set_kernel_watch_mem(&(lock->mem), k_autolock_mem);
#endif
    return I_FAILURE;
}

static int32_t
LOCK_BASE_MAKE_NAME(I_user_lock_base_lock)(
    LOCK_BASE_MAKE_NAME(I_user_lock_base_t) * lock) {
    int32_t                      iter_count, i, backoff_iter;
    struct kernel_autolock_abi * k_autolock_mem;

    /* Allow these to be unused as some of them are config
     * dependendent. */
    (void)(iter_count);
    (void)(i);
    (void)(backoff_iter);
    (void)(k_autolock_mem);

    enum {
        PAUSE_ITERS = 128,
        YIELD_ITERS = 4,

        BACKOFF_MAX_MASK  = 255 /* 255 so we can just use movzbl. */,
        BACKOFF_INIT_MASK = 31
    };

    if (LOCK_BASE_MAKE_NAME(I_user_lock_base_trylock)(lock) ==
        I_SUCCESS) {
        return I_SUCCESS;
    }

#if I_WITH_AUTOLOCK
    k_autolock_mem = autolock_init_kernel_state();


    /* Initialize kernel for this backoff_autolock. */

    /* What for unlocked. */
    autolock_set_kernel_watch_for(LOCK_BASE_MAKE_NAME(I_UNLOCKED),
                                  k_autolock_mem);

    /* Schedule when equals. */
    autolock_set_kernel_watch_neq(0, k_autolock_mem);
#endif


#if I_WITH_BACKOFF
    backoff_iter = get_ll_time() & BACKOFF_INIT_MASK;
#endif

#if I_WITH_FUTEX
restart:
    (void)(0);
#endif

#if I_WITH_YIELD || I_WITH_FUTEX
    iter_count = 0;
#endif

    for (;;) {
        /* I_backoff_autolock_trylock_maybe_sched will manage enabling /
         * disabling autolock in a safe manner. */
        if (LOCK_BASE_MAKE_NAME(
                I_internal_user_lock_base_trylock_maybe_sched)(
                lock, k_autolock_mem) == I_SUCCESS) {
            /* watch_mem will be NULL. */
            return I_SUCCESS;
        }

#if I_WITH_BACKOFF
        /* pause to avoid runaway speculation. Note backoff could be
         * added VERY efficiently if we paired this with rseq. */
        for (i = backoff_iter; i; --i) {
            ll_pause();
        }

        /* Wasted work if we acquire lock next iteration. */
        backoff_iter += (backoff_iter + 1);
        backoff_iter &= BACKOFF_MAX_MASK;
#else
        ll_pause();
#endif


#if I_WITH_YIELD || I_WITH_FUTEX
        /* If we have some "better" wait mechanism then test iter count.
         */
        if (UNLIKELY(--iter_count < (-PAUSE_ITERS))) {
            break;
        }
#endif
    }

#if I_WITH_YIELD
#if I_WITH_FUTEX
    iter_count = 0;
#endif
    for (;;) {
        yield();
        if (LOCK_BASE_MAKE_NAME(
                I_internal_user_lock_base_trylock_maybe_sched)(
                lock, k_autolock_mem) == I_SUCCESS) {
            /* watch_mem will be NULL. */
            return I_SUCCESS;
        }
#if I_WITH_FUTEX
        /* If we have some "better" wait mechanism then test iter count.
         */
        if (UNLIKELY(--iter_count < (-YIELD_ITERS))) {
            break;
        }
#endif
    }

#endif

#if I_WITH_FUTEX
    /* Counter so unlock knows if it needs to wake a thread.
     */
    __atomic_fetch_add(&(lock->waiters), 1, __ATOMIC_RELAXED);

    /* We can silently ignore futex errors and still be
     * correct. */
    futex_wait(&(lock->mem), LOCK_BASE_MAKE_NAME(I_LOCKED));

    /* dec counter. Its safe to call futex_unlock with no
     * threads waiting but ultimately adds a syscall to
     * unlock overhead so best to avoid it if possible. */
    __atomic_fetch_sub(&(lock->waiters), 1, __ATOMIC_RELAXED);
    goto restart;
#endif

    __builtin_unreachable();
}

extern_C_end();

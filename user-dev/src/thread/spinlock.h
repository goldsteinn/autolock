#ifndef _SRC__THREAD__SPINLOCK_H_
#define _SRC__THREAD__SPINLOCK_H_


#ifndef WITH_THREAD
#error "Using thread library with threading disabled!"
#endif

#include <pthread.h>

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"


typedef pthread_spinlock_t thread_spinlock_t;

#define safe_thread_spinlock_init(spinlock, pshared)                           \
    I_safe_thread_spinlock_init(spinlock, pshared, ERR_ARGS)
#define safe_thread_spinlock_destroy(spinlock)                                 \
    I_safe_thread_spinlock_destroy(spinlock, ERR_ARGS)

#define safe_thread_spinlock_lock(spinlock)                                    \
    I_safe_thread_spinlock_lock(spinlock, ERR_ARGS)
#define safe_thread_spinlock_unlock(spinlock)                                  \
    I_safe_thread_spinlock_unlock(spinlock, ERR_ARGS)
#define safe_thread_spinlock_trylock(spinlock)                                 \
    I_safe_thread_spinlock_trylock(spinlock, ERR_ARGS)


static NONNULL(1) void I_safe_thread_spinlock_init(
    thread_spinlock_t * restrict spinlock,
    int32_t pshared,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_spin_init(spinlock, pshared))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_spinlock_destroy(
    thread_spinlock_t * restrict spinlock,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_spin_destroy(spinlock))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) int32_t thread_spinlock_lock(thread_spinlock_t * spinlock) {
    return pthread_spin_lock(spinlock);
}

static NONNULL(1) int32_t thread_spinlock_unlock(thread_spinlock_t * spinlock) {
    return pthread_spin_unlock(spinlock);
}

static NONNULL(1) int32_t
    thread_spinlock_trylock(thread_spinlock_t * spinlock) {
    return pthread_spin_trylock(spinlock);
}


static NONNULL(1) void I_safe_thread_spinlock_lock(
    thread_spinlock_t * restrict spinlock,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(thread_spinlock_lock(spinlock))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_spinlock_unlock(
    thread_spinlock_t * restrict spinlock,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(thread_spinlock_unlock(spinlock))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) int32_t
    I_safe_thread_spinlock_trylock(thread_spinlock_t * restrict spinlock,
                                   char const * restrict fn,
                                   char const * restrict func,
                                   uint32_t ln) {
    int32_t ret = thread_spinlock_unlock(spinlock);
    if (UNLIKELY(ret && (ret != EBUSY))) {
        I_errdie(fn, func, ln, errno, NULL);
    }

    return ret;
}


#endif

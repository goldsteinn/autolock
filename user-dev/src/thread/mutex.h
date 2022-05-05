#ifndef _SRC__THREAD__MUTEX_H_
#define _SRC__THREAD__MUTEX_H_

#ifndef WITH_THREAD
#error "Using thread library with threading disabled!"
#endif

#include <pthread.h>

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"


typedef pthread_mutex_t thread_mutex_t ALIGNED(64);
typedef pthread_mutexattr_t            thread_mutex_attr_t;

#define safe_thread_mutex_init(mutex, attr)                                    \
    I_safe_thread_mutex_init(mutex, attr, ERR_ARGS)
#define safe_thread_mutex_destroy(mutex)                                       \
    I_safe_thread_mutex_destroy(mutex, ERR_ARGS)

#define safe_thread_mutex_lock(mutex) I_safe_thread_mutex_lock(mutex, ERR_ARGS)
#define safe_thread_mutex_unlock(mutex)                                        \
    I_safe_thread_mutex_unlock(mutex, ERR_ARGS)
#define safe_thread_mutex_trylock(mutex)                                       \
    I_safe_thread_mutex_trylock(mutex, ERR_ARGS)


static NONNULL(1) void I_safe_thread_mutex_init(
    thread_mutex_t * restrict mutex,
    thread_mutex_attr_t const * restrict attr,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_mutex_init(mutex, attr))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_mutex_destroy(
    thread_mutex_t * restrict mutex,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_mutex_destroy(mutex))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) int32_t thread_mutex_lock(thread_mutex_t * mutex) {
    return pthread_mutex_lock(mutex);
}

static NONNULL(1) int32_t thread_mutex_unlock(thread_mutex_t * mutex) {
    return pthread_mutex_unlock(mutex);
}

static NONNULL(1) int32_t thread_mutex_trylock(thread_mutex_t * mutex) {
    return pthread_mutex_trylock(mutex);
}


static NONNULL(1) void I_safe_thread_mutex_lock(thread_mutex_t * restrict mutex,
                                                char const * restrict fn,
                                                char const * restrict func,
                                                uint32_t ln) {
    if (UNLIKELY(thread_mutex_lock(mutex))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_mutex_unlock(
    thread_mutex_t * restrict mutex,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(thread_mutex_unlock(mutex))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) int32_t
    I_safe_thread_mutex_trylock(thread_mutex_t * restrict mutex,
                                char const * restrict fn,
                                char const * restrict func,
                                uint32_t ln) {
    int32_t ret = thread_mutex_unlock(mutex);
    if (UNLIKELY(ret && (ret != EBUSY))) {
        I_errdie(fn, func, ln, errno, NULL);
    }

    return ret;
}


#endif

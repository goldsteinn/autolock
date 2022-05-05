#ifndef _SRC__THREAD__BARRIER_H_
#define _SRC__THREAD__BARRIER_H_

#ifndef WITH_THREAD
#error "Using thread library with threading disabled!"
#endif

#include <pthread.h>

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"


typedef pthread_barrier_t thread_barrier_t ALIGNED(64);
typedef pthread_barrierattr_t              thread_barrier_attr_t;

enum { THREAD_BARRIER_IS_UNIQUE = PTHREAD_BARRIER_SERIAL_THREAD };

#define safe_thread_barrier_init(barrier, attr, N)                             \
    I_safe_thread_barrier_init(barrier, attr, N, ERR_ARGS)
#define safe_thread_barrier_destroy(barrier)                                   \
    I_safe_thread_barrier_destroy(barrier, ERR_ARGS)

#define safe_thread_barrier_wait(barrier)                                      \
    I_safe_thread_barrier_wait(barrier, ERR_ARGS)


static NONNULL(1) void I_safe_thread_barrier_init(
    thread_barrier_t * restrict barrier,
    thread_barrier_attr_t const * restrict attr,
    uint32_t nthreads,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_barrier_init(barrier, attr, nthreads))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_barrier_destroy(
    thread_barrier_t * restrict barrier,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_barrier_destroy(barrier))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) int32_t thread_barrier_wait(thread_barrier_t * barrier) {
    return pthread_barrier_wait(barrier);
}

static NONNULL(1) void I_safe_thread_barrier_wait(
    thread_barrier_t * restrict barrier,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(thread_barrier_wait(barrier))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}


#endif

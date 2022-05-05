#ifndef _SRC__THREAD__THREAD_H_
#define _SRC__THREAD__THREAD_H_


#ifndef WITH_THREAD
#error "Using thread library with threading disabled!"
#endif

#include <pthread.h>

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"

#include "thread/cpuset.h"

typedef pthread_t      thread_t;
typedef pthread_attr_t thread_attr_t;

typedef void * (*I_thread_func)(void *);

#define safe_thread_create(tid, attr, func, arg)                               \
    I_safe_thread_create(tid, attr, func, arg, ERR_ARGS)
#define safe_thread_join(tid, attr) I_safe_thread_join(tid, attr, ERR_ARGS)

#define safe_thread_attr_init(attr) I_safe_thread_attr_init(attr, ERR_ARGS)
#define safe_thread_attr_destroy(attr)                                         \
    I_safe_thread_attr_destroy(attr, ERR_ARGS)
#define safe_thread_attr_set_stacksize(attr, stacksize)                        \
    I_safe_thread_attr_set_stacksize(attr, stacksize, ERR_ARGS)
#define safe_thread_attr_set_affinity(attr, cpuset)                            \
    I_safe_thread_attr_set_affinity(attr, cpuset, ERR_ARGS)

static NONNULL(1, 3) void I_safe_thread_create(thread_t * restrict tid,
                                               thread_attr_t * restrict attr,
                                               I_thread_func thread_func,
                                               void * restrict arg,
                                               char const * restrict fn,
                                               char const * restrict func,
                                               uint32_t ln) {
    if (UNLIKELY(pthread_create(tid, attr, thread_func, arg))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static void
I_safe_thread_join(thread_t tid,
                   void * restrict * restrict retval,
                   char const * restrict fn,
                   char const * restrict func,
                   uint32_t ln) {
    if (UNLIKELY(pthread_join(tid, retval))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_attr_init(thread_attr_t * restrict attr,
                                               char const * restrict fn,
                                               char const * restrict func,
                                               uint32_t ln) {
    if (UNLIKELY(pthread_attr_init(attr))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}

static NONNULL(1) void I_safe_thread_attr_destroy(thread_attr_t * restrict attr,
                                                  char const * restrict fn,
                                                  char const * restrict func,
                                                  uint32_t ln) {
    if (UNLIKELY(pthread_attr_destroy(attr))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}


static NONNULL(1) void I_safe_thread_attr_set_stacksize(
    thread_attr_t * restrict attr,
    uint32_t stacksize,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(pthread_attr_setstacksize(attr, stacksize))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}


static NONNULL(1) void I_safe_thread_attr_set_affinity(
    thread_attr_t * restrict attr,
    cpuset_t const * restrict cpu_set,
    char const * restrict fn,
    char const * restrict func,
    uint32_t ln) {
    if (UNLIKELY(
            pthread_attr_setaffinity_np(attr, sizeof(cpuset_t), cpu_set))) {
        I_errdie(fn, func, ln, errno, NULL);
    }
}


#endif

#ifndef _SRC__INTERPOSE__PTHREAD_MUTEX_INTERPOSE_H_
#define _SRC__INTERPOSE__PTHREAD_MUTEX_INTERPOSE_H_

#include "util/common.h"
extern_C_start();

#include "interpose/interpose-timing.h"
#include "interpose/pthread-mutex-api.h"

/* Must do this in C (as opposed to templates) for C-style linkage. */


#define I_pthread_func_name(lock_name, pthread_func)                   \
    CAT(I_, lock_name, _, pthread_func)

#define I_lock_t(lock_name)         CAT(lock_name, _t)
#define I_lockattr_t(lock_name)     CAT(lock_name, attr, _t)
#define I_lock_init_f(lock_name)    CAT(lock_name, _init)
#define I_lock_destroy_f(lock_name) CAT(lock_name, _destroy)
#define I_lock_trylock_f(lock_name) CAT(lock_name, _trylock)
#define I_lock_lock_f(lock_name)    CAT(lock_name, _lock)
#define I_lock_unlock_f(lock_name)  CAT(lock_name, _unlock)


#define I_create_pthread_api(lock_name, lock_t, lock_init_f,           \
                             lock_destroy_f, lock_trylock_f,           \
                             lock_lock_f, lock_unlock_f)               \
                                                                       \
                                                                       \
    const_assert(sizeof(I_lock_t(lock_name)) <=                        \
                 sizeof(pthread_mutex_t));                             \
                                                                       \
    typedef union __attribute__((may_alias)) I_pthread_mutex {         \
        pthread_mutex_t glibc_mutex;                                   \
        I_lock_t(lock_name) custom_mutex;                              \
    } I_pthread_mutex_t;                                               \
                                                                       \
    I_make_I_get_lock_name(lock_name);                                 \
                                                                       \
    static int I_pthread_mutex_init(                                   \
        pthread_mutex_t *           mutex,                             \
        const pthread_mutexattr_t * mutexattr) {                       \
        int                 ret;                                       \
        I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;      \
                                                                       \
        /* Leave unused for now. */                                    \
        (void)(mutexattr);                                             \
                                                                       \
        ret = lock_init_f(&(mutex_u->custom_mutex));                   \
                                                                       \
        return ret;                                                    \
    }                                                                  \
                                                                       \
    static int I_pthread_mutex_destroy(pthread_mutex_t * mutex) {      \
        int                 ret;                                       \
        I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;      \
                                                                       \
        ret = lock_destroy_f(&(mutex_u->custom_mutex));                \
                                                                       \
        return ret;                                                    \
    }                                                                  \
                                                                       \
    static int I_pthread_mutex_trylock(pthread_mutex_t * mutex) {      \
        int                 ret;                                       \
        I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;      \
                                                                       \
        timing_start(trylock);                                         \
        ret = lock_trylock_f(&(mutex_u->custom_mutex));                \
        timing_end(trylock);                                           \
                                                                       \
        return ret;                                                    \
    }                                                                  \
                                                                       \
    static int I_pthread_mutex_lock(pthread_mutex_t * mutex) {         \
        int                 ret;                                       \
        I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;      \
                                                                       \
        timing_start(lock);                                            \
        ret = lock_lock_f(&(mutex_u->custom_mutex));                   \
        timing_end(lock);                                              \
                                                                       \
        return ret;                                                    \
    }                                                                  \
                                                                       \
    static int I_pthread_mutex_unlock(pthread_mutex_t * mutex) {       \
        int                 ret;                                       \
        I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;      \
                                                                       \
        timing_start(unlock);                                          \
        ret = lock_unlock_f(&(mutex_u->custom_mutex));                 \
        timing_end_unlock(unlock);                                     \
                                                                       \
        return ret;                                                    \
    }


#define I_create_pthread_alias()                                       \
    typedef_func(pthread_mutex_init, I_pthread_mutex_init);            \
    typedef_func(pthread_mutex_destroy, I_pthread_mutex_destroy);      \
    typedef_func(pthread_mutex_trylock, I_pthread_mutex_trylock);      \
    typedef_func(pthread_mutex_lock, I_pthread_mutex_lock);            \
    typedef_func(pthread_mutex_unlock, I_pthread_mutex_unlock);


#define custom_create_pthread_interpose(...)                           \
    I_create_pthread_api(__VA_ARGS__);                                 \
    I_create_pthread_alias();

#define create_pthread_interpose(lock_name)                            \
    custom_create_pthread_interpose(                                   \
        lock_name, I_lock_t(lock_name), I_lock_init_f(lock_name),      \
        I_lock_destroy_f(lock_name), I_lock_trylock_f(lock_name),      \
        I_lock_lock_f(lock_name), I_lock_unlock_f(lock_name))

extern_C_end();
#endif

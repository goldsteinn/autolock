#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>

#include "interpose/pthread-mutex-interpose.h"
#include "util/types.h"


/* Since we are interposing the pthread_mutex symbols need to manually
 * get the proper functions. */
static FUNC_T(pthread_mutex_init) I_pthread_mutex_init_fp;
static FUNC_T(pthread_mutex_destroy) I_pthread_mutex_destroy_fp;
static FUNC_T(pthread_mutex_trylock) I_pthread_mutex_trylock_fp;
static FUNC_T(pthread_mutex_lock) I_pthread_mutex_lock_fp;
static FUNC_T(pthread_mutex_unlock) I_pthread_mutex_unlock_fp;

static __attribute__((constructor)) void
I_init_pthread_mutex() {
    I_pthread_mutex_init_fp = CAST_TO_FUNC(
        pthread_mutex_init, dlsym(RTLD_NEXT, "pthread_mutex_init"));

    I_pthread_mutex_destroy_fp =
        CAST_TO_FUNC(pthread_mutex_destroy,
                     dlsym(RTLD_NEXT, "pthread_mutex_destroy"));

    I_pthread_mutex_trylock_fp =
        CAST_TO_FUNC(pthread_mutex_trylock,
                     dlsym(RTLD_NEXT, "pthread_mutex_trylock"));

    I_pthread_mutex_lock_fp = CAST_TO_FUNC(
        pthread_mutex_lock, dlsym(RTLD_NEXT, "pthread_mutex_lock"));

    I_pthread_mutex_unlock_fp = CAST_TO_FUNC(
        pthread_mutex_unlock, dlsym(RTLD_NEXT, "pthread_mutex_unlock"));

    assert(I_pthread_mutex_init_fp != NULL);
    assert(I_pthread_mutex_destroy_fp != NULL);
    assert(I_pthread_mutex_trylock_fp != NULL);
    assert(I_pthread_mutex_lock_fp != NULL);
    assert(I_pthread_mutex_unlock_fp != NULL);
}

static __attribute__((destructor)) void
I_destroy_pthread_mutex() {
    /* nop. */
}


#define I_pthread_mutex_init_w(mutex)                                  \
    I_pthread_mutex_init_fp(mutex, NULL)


custom_create_pthread_interpose(pthread_mutex,
                                pthread_mutex_t,
                                I_pthread_mutex_init_w,
                                I_pthread_mutex_destroy_fp,
                                I_pthread_mutex_trylock_fp,
                                I_pthread_mutex_lock_fp,
                                I_pthread_mutex_unlock_fp);

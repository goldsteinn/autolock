#ifndef _SRC__LOCKS__SIMPLE_AUTOLOCK_H_
#define _SRC__LOCKS__SIMPLE_AUTOLOCK_H_

/* Where the common user-level autolock features are implemented. This
 * includes the lock type, and init/destroy/unlock/trylock functions. */
#include "internal/autolock-common-user-api.h"

/* The user-level lock type and functions {init|destroy|trylock|unlock}
 * are all essentially unchanging so just use alias them to common
 * defintions. */
typedef I_user_autolock_t simple_autolock_t;
static typedef_func(simple_autolock_init, I_user_autolock_init);
static typedef_func(simple_autolock_destroy, I_user_autolock_destroy);
static typedef_func(simple_autolock_trylock, I_user_autolock_trylock);
static typedef_func(simple_autolock_unlock, I_user_autolock_unlock);

/* Could do the above with a macro but its not that cumbersome and the
 * readability value of seeing the API defined explicitly here
 * outweights. */

static NONNULL(1) int32_t
    simple_autolock_lock(simple_autolock_t * lock) {
    struct kernel_autolock_abi * k_autolock_mem;
    /* Uncontended just grab the lock. No intialization is needed. */
    if (simple_autolock_trylock(lock) == I_SUCCESS) {
        return I_SUCCESS;
    }
    k_autolock_mem = autolock_init_kernel_state();


    /* Initialize kernel for this simple_autolock. */

    /* What for unlocked. */
    autolock_set_kernel_watch_for(I_UNLOCKED, k_autolock_mem);

    /* Schedule when equals. */
    autolock_set_kernel_watch_neq(0, k_autolock_mem);
    for (;;) {
        /* I_simple_autolock_trylock_maybe_sched will manage enabling /
         * disabling autolock in a safe manner. */
        if (I_internal_user_autolock_trylock_maybe_sched(
                lock, k_autolock_mem) == 0) {
            /* watch_mem will be NULL. */
            return I_SUCCESS;
        }
        /* pause to avoid runaway speculation. Note backoff could be
         * added VERY efficiently if we paired this with rseq. */
        ll_pause();
    }
}


#endif

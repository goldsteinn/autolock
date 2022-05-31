#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__CUSTOM_BACKOFF_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__CUSTOM_BACKOFF_AUTOLOCK_H_

/* Where the common user-level autolock features are implemented. This
 * includes the lock type, and init/destroy/unlock/trylock functions. */
#include "internal/autolock-common-user-api.h"

#include "arch/ll-timing.h"

/* The user-level lock type and functions {init|destroy|trylock|unlock}
 * are all essentially unchanging so just use alias them to common
 * defintions. */
typedef I_user_autolock_t custom_backoff_autolock_t;
static typedef_func(custom_backoff_autolock_init, I_user_autolock_init);
static typedef_func(custom_backoff_autolock_destroy,
                    I_user_autolock_destroy);
static typedef_func(custom_backoff_autolock_trylock,
                    I_user_autolock_trylock);
static typedef_func(custom_backoff_autolock_unlock,
                    I_user_autolock_unlock);


static NONNULL(1) int32_t
    custom_backoff_autolock_lock(custom_backoff_autolock_t * lock) {
    struct kernel_autolock_abi * k_autolock_mem;
    uint32_t                     custom_backoff_iter;
    enum {
        CUSTOM_BACKOFF_MAX_MASK =
            255 /* 255 so we can just use movzbl. */,
        CUSTOM_BACKOFF_INIT_MASK = 31
    };

    /* Uncontended just grab the lock. No intialization is needed. */
    if (custom_backoff_autolock_trylock(lock) == I_SUCCESS) {
        return I_SUCCESS;
    }
    k_autolock_mem = autolock_init_kernel_state();


    /* Initialize kernel for this custom_backoff_autolock. */

    /* What for unlocked. */
    autolock_set_kernel_watch_for(I_UNLOCKED, k_autolock_mem);

    /* Schedule when equals. */
    autolock_set_kernel_watch_neq(0, k_autolock_mem);

    custom_backoff_iter = get_ll_time() & CUSTOM_BACKOFF_INIT_MASK;
    for (;;) {
        uint32_t i;
        /* I_custom_backoff_autolock_trylock_maybe_sched will manage
         * enabling / disabling autolock in a safe manner. */
        if (I_internal_user_autolock_trylock_maybe_sched(
                lock, k_autolock_mem) == I_SUCCESS) {
            /* watch_mem will be NULL. */
            return I_SUCCESS;
        }


        /* pause to avoid runaway speculation. Note custom_backoff could
         * be added VERY efficiently if we paired this with rseq. */
        for (i = custom_backoff_iter; i; --i) {
            ll_pause();
        }

        /* Wasted work if we acquire lock next iteration. */
        custom_backoff_iter += (custom_backoff_iter + 1);
        custom_backoff_iter &= CUSTOM_BACKOFF_MAX_MASK;
    }
}


#endif

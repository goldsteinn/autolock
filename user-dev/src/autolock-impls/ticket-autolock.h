#ifndef _SRC__AUTOLOCK_IMPLS__TICKET_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__TICKET_AUTOLOCK_H_

/* Need to implement autolock base custom. */
#include "arch/ll-pause.h"
#include "autolock-impls/autolock-kernel-api.h"
#include "autolock-impls/internal/autolock-common-consts.h"


typedef struct I_ticket_autolock {
    uint32_t cur_count;
    /* Padd to cache line to `cur_count` line is not being ping-ponged
     * as threads busy-wait on it. */
    uint8_t  padding[64 - sizeof(uint32_t) * 1];
    uint32_t next_count;
} ticket_autolock_t __attribute__((aligned(64)));


/* Zero relevant fields and init kernel state. */
static NONNULL(1) int32_t
    ticket_autolock_init(ticket_autolock_t * lock) {
    lock->next_count = 0;
    lock->cur_count  = 0;


    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }

    /* Don't let compiler implement this with cmov. */
    asm volatile("" : : :);
    return I_SUCCESS;
}

/* Nothing to do for destroy. */
static NONNULL(1) int32_t
    ticket_autolock_destroy(ticket_autolock_t * lock) {
    (void)(lock);
    return autolock_release_kernel_state();
}


static NONNULL(1) int32_t
    ticket_autolock_trylock(ticket_autolock_t * lock) {
    uint32_t ticket_num =
        __atomic_load_n(&(lock->next_count), __ATOMIC_RELAXED);
    uint32_t cur_num =
        __atomic_load_n(&(lock->cur_count), __ATOMIC_RELAXED);

    if (ticket_num == cur_num) {
        uint32_t next_ticket_num = ticket_num + 1;
        /* Slower than `lock xadd` but necessary so we don't modify
         * state w.o acquiring. */
        return !__atomic_compare_exchange_n(
            &(lock->next_count), &ticket_num, next_ticket_num,
            1 /* weak. */, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
    return 1;
}
static NONNULL(1) int32_t
    ticket_autolock_unlock(ticket_autolock_t * lock) {
    lock->cur_count += 1;
    return 0;
}


static NONNULL(1) int32_t
    ticket_autolock_lock(ticket_autolock_t * lock) {
    struct kernel_autolock_abi * k_autolock_mem;

    /* Get count we need. Note we use fetch add here so we are getting
     * `next_count` BEFORE the incr. */
    uint32_t ticket_num =
        __atomic_fetch_add(&(lock->next_count), 1, __ATOMIC_RELAXED);

    /* Get current count. */
    uint32_t cur_num =
        __atomic_load_n(&(lock->cur_count), __ATOMIC_RELAXED);


    /* If we are at `cur_count` then done. */
    if (LIKELY(ticket_num == cur_num)) {
        return 0;
    }


    /* Initialize kernel state. */
    k_autolock_mem = autolock_init_kernel_state();

    /* For ticket lock we don't need to go back and forth between
     * arming/disarming the autolock as once the thread acquires the
     * lock lock->cur_count will equal ticket_num so the thread will
     * still be schedulable. */
    autolock_set_kernel_watch_mem(&(lock->cur_count), k_autolock_mem);
    autolock_set_kernel_watch_for(ticket_num, k_autolock_mem);
    autolock_set_kernel_watch_neq(0, k_autolock_mem);

    for (;;) {
        uint32_t backoff;
        int32_t  s_cur_num;
        cur_num = __atomic_load_n(&(lock->cur_count), __ATOMIC_RELAXED);

        /* NB: This will become `subl %[ticket_num], %[cur_num]; je`
         * which is just as efficient as `cmpl ...`. */
        cur_num -= ticket_num;
        if (cur_num == 0) {
            /* Disarm autolock before returning. */
            autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
            return 0;
        }

        /* Always pause to prohibit runaway speculation. */
        ll_pause();


        s_cur_num = cur_num;
        /* If we are far enough back on queue use some form of backoff.
         */
        if (s_cur_num <= (-1)) {
            /* It will be a while to desched self. */
            if (s_cur_num <= (-8)) {
                yield();
            }
            else {
                /* Not super far back so just backoff loop for a bit.
                 * Scale backoff loop duration with queue number. */
                backoff = 1
                          << ((-cur_num) & (sizeof_bits(backoff) - 1));
                for (; backoff; --backoff) {
                    ll_pause();
                }
            }
        }
    }
}


#endif

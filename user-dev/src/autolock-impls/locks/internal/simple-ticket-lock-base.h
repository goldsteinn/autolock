extern_C_start();

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__SIMPLE_TICKET_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__SIMPLE_TICKET_LOCK_BASE_H_


/* Simple ticket spinlock. Naive and performs horribly once nthreads >
 * ncores. */

#ifndef I_WITH_AUTOLOCK
#error "Must define I_WITH_AUTOLOCK before including ticket base"
#endif

#include "arch/ll-pause.h"
#include "util/attrs.h"
#include "util/cpu-info.h"

/* Need to implement autolock base custom. */
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/sys/autolock-kernel-api.h"


typedef struct I_simple_ticket_lock_base {
    union {
        uint32_t cur_count;
        /* Padd to cache line to `cur_count` line is not being
         * ping-ponged as threads busy-wait on it. */
        uint8_t I_padding[L1_CACHE_LINE_SIZE - sizeof(uint32_t) * 1];
    };
    uint32_t next_count;
} simple_ticket_lock_base_t ALIGNED(L1_CACHE_LINE_SIZE);


/* Nothing to do for destroy. */
static NONNULL(1) int32_t I_simple_ticket_lock_base_destroy(
    simple_ticket_lock_base_t * lock) {
    (void)(lock);
    return autolock_release_kernel_state();
}


static NONNULL(1) int32_t I_simple_ticket_lock_base_trylock(
    simple_ticket_lock_base_t * lock) {
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
    I_simple_ticket_lock_base_unlock(simple_ticket_lock_base_t * lock) {
    lock->cur_count += 1;
    return 0;
}
#endif

/* Zero relevant fields and init kernel state. */
static NONNULL(1) int32_t
    CAT(I_simple_ticket_lock_base_init,
        I_WITH_AUTOLOCK)(simple_ticket_lock_base_t * lock) {
    lock->next_count = 0;
    lock->cur_count  = 0;

#if I_WITH_AUTOLOCK
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
#endif

    /* Don't let compiler implement this with cmov. */
    asm volatile("" : : :);
    return I_SUCCESS;
}


static NONNULL(1) int32_t
    CAT(I_simple_ticket_lock_base_lock,
        I_WITH_AUTOLOCK)(simple_ticket_lock_base_t * lock) {


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
#if I_WITH_AUTOLOCK
    struct kernel_autolock_abi * k_autolock_mem;
    /* Initialize kernel state. */
    k_autolock_mem = autolock_init_kernel_state();


    /* For ticket lock we don't need to go back and forth between
     * arming/disarming the autolock as once the thread acquires the
     * lock lock->cur_count will equal ticket_num so the thread will
     * still be schedulable. */
    autolock_set_kernel_watch_mem(&(lock->cur_count), k_autolock_mem);
    autolock_set_kernel_watch_for(ticket_num, k_autolock_mem);
    autolock_set_kernel_watch_neq(0, k_autolock_mem);
#endif
    for (;;) {
        uint32_t backoff;
        int32_t  s_cur_num;
        cur_num = __atomic_load_n(&(lock->cur_count), __ATOMIC_RELAXED);

        /* NB: This will become `subl %[ticket_num], %[cur_num]; je`
         * which is just as efficient as `cmpl ...`. */
        cur_num -= ticket_num;
        if (cur_num == 0) {
#if I_WITH_AUTOLOCK
            /* Disarm autolock before returning. */
            autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
            return I_SUCCESS;
        }

        /* Always pause to prohibit runaway speculation. */
        ll_pause();


        s_cur_num = cur_num;
        /* If we are far enough back on queue use some form of backoff.
         */
        if (s_cur_num <= (-1)) {
            /* It will be a while to desched self. */
            if (s_cur_num <= (-2)) {
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

extern_C_end();

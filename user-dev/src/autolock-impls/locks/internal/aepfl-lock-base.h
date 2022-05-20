
/*
 * T. E. Anderson. 1990.
 * The Performance of Spin Lock Alternatives for Shared-Memory
 * Multiprocessors. IEEE Trans. Parallel Distrib. Syst. 1, 1 (January
 * 1990).
 *
 * The implementation is largely taken from libslock.
 * https://github.com/tudordavid/libslock/blob/master/src/alock.c
 *
 * Lock design summary:
 * - There is one slot for each thread in a fixed-size array
 * - When a thread wants to lock, it get its slot number via an atomic
 * increment
 * - The thread spins (if needed) on the memory address of its slot
 * - On unlock, the thread wakes the thread of the slot next to its own
 */

#include "arch/ll-prefetch.h"
#include "util/attrs.h"
#include "util/cpu-info.h"

extern_C_start();
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/locks/internal/lock-wait-common.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__AEPFL_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__AEPFL_LOCK_BASE_H_


/********************************************************************/
/* Lock constants. */
enum {
    I_AEPFL_UNLOCKED    = 1,
    I_AEPFL_LOCKED      = 0,
    I_AEPFL_PAUSE_ITERS = 128,
    I_AEPFL_FLAG_BYTES  = 1,
    I_AEPFL_MAX_THREADS = 128,
    I_AEPFL_PAUSE_BOUND = 2,

};

/********************************************************************/
/* Internal flag struct. */

typedef uint32_t I_aepfl_mem_flag_t;
typedef struct I_aepfl_flag {
    union {
        I_aepfl_mem_flag_t mem;
        /* Resize struct test change levels of interference. */
        uint8_t padding[I_AEPFL_FLAG_BYTES];
    } ALIGNED(CMAX((size_t)I_AEPFL_FLAG_BYTES,
                   sizeof(I_aepfl_mem_flag_t)));
} I_aepfl_flag_t ALIGNED(CMAX((size_t)I_AEPFL_FLAG_BYTES,
                              sizeof(I_aepfl_mem_flag_t)));

typedef struct I_aepfl_lock_base {
    union {
        uint32_t tail;
        uint8_t  padding0[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
    union {
        uint32_t head;
        uint8_t  padding1[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);

    I_aepfl_flag_t flags[I_AEPFL_MAX_THREADS] ALIGNED(
        L1_CACHE_LINE_SIZE);

} I_aepfl_lock_base_t ALIGNED(L1_CACHE_LINE_SIZE);

/********************************************************************/
/* Interal api. */
static NONNULL(1) uint32_t
    I_aepfl_lock_base_fetch_add_slot(uint32_t * slot_mem);
static uint32_t I_aepfl_lock_base_get_next_slot(uint32_t slot);
static NONNULL(1) I_aepfl_mem_flag_t * I_aepfl_lock_base_get_next(
    I_aepfl_flag_t * flags,
    uint32_t         slot);


/********************************************************************/
/* aepfl lock base api. */
static NONNULL(1) int32_t
    I_aepfl_lock_base_init(I_aepfl_lock_base_t * lock);
static NONNULL(1) int32_t
    I_aepfl_lock_base_destroy(I_aepfl_lock_base_t * lock);
static NONNULL(1) int32_t
    I_aepfl_lock_base_trylock(I_aepfl_lock_base_t * lock);
/* lock api below. */
static NONNULL(1) int32_t
    I_aepfl_lock_base_unlock(I_aepfl_lock_base_t * lock);


/********************************************************************/
/* api implementation. */
static int32_t
I_aepfl_lock_base_init(I_aepfl_lock_base_t * lock) {
    lock->tail = 0;
    lock->head = 0;

    /* memset performs better when you give it power of 2 so just
     * duplicate the write. */
    __builtin_memset(lock->flags, I_AEPFL_LOCKED,
                     I_AEPFL_MAX_THREADS * sizeof(I_aepfl_flag_t));

    lock->flags[0].mem = I_AEPFL_UNLOCKED;
    return I_SUCCESS;
}

static int32_t
I_aepfl_lock_base_destroy(I_aepfl_lock_base_t * lock) {
    (void)(lock);
    return I_SUCCESS;
}

static int32_t
I_aepfl_lock_base_trylock(I_aepfl_lock_base_t * lock) {
    uint32_t cur_tail =
        __atomic_load_n(&(lock->tail), __ATOMIC_RELAXED);
    uint32_t cur_head =
        __atomic_load_n(&(lock->tail), __ATOMIC_RELAXED);

    if (LIKELY(cur_tail == cur_head)) {
        if (LIKELY(__atomic_compare_exchange_n(
                &(lock->tail), &cur_tail, cur_tail + 1, true,
                __ATOMIC_RELAXED, __ATOMIC_RELAXED))) {
            return I_SUCCESS;
        }
    }
    return I_FAILURE;
}


static int32_t
I_aepfl_lock_base_unlock(I_aepfl_lock_base_t * lock) {
    uint32_t slot;
    ll_prefetchw(&(lock->head));
    ll_prefetchw(&(lock->flags));

    slot = I_aepfl_lock_base_fetch_add_slot(&(lock->head));
    __atomic_store_n(I_aepfl_lock_base_get_next(lock->flags, slot),
                     I_AEPFL_LOCKED, __ATOMIC_RELAXED);

    slot = I_aepfl_lock_base_get_next_slot(slot);
    __atomic_store_n(I_aepfl_lock_base_get_next(lock->flags, slot),
                     I_AEPFL_UNLOCKED, __ATOMIC_RELAXED);
    return I_SUCCESS;
}

/********************************************************************/
/* Internal API implementation. */

/* NB: Especially on older CPUs it will be beneficial to replace the add
 * by 1 with sizeof(I_aepfl_mem_flag_t) and do the AGU directly to avoid
 * complex address mode. */
static uint32_t
I_aepfl_lock_base_fetch_add_slot(uint32_t * slot_mem) {
    uint32_t slot = __atomic_fetch_add(slot_mem, 1, __ATOMIC_RELAXED);
    return slot % I_AEPFL_MAX_THREADS;
}

static uint32_t
I_aepfl_lock_base_get_next_slot(uint32_t slot) {
    return (slot + 1) % I_AEPFL_MAX_THREADS;
}

static I_aepfl_mem_flag_t *
I_aepfl_lock_base_get_next(I_aepfl_flag_t * flags, uint32_t slot) {
    return &(flags[slot].mem);
}

#endif

static NONNULL(1) int32_t
    CAT(I_aepfl_lock_base_lock,
        I_WITH_AUTOLOCK)(I_aepfl_lock_base_t * lock);


static int32_t
CAT(I_aepfl_lock_base_lock,
    I_WITH_AUTOLOCK)(I_aepfl_lock_base_t * lock) {
    uint32_t             slot;
    I_aepfl_mem_flag_t * flag_mem;
    ll_prefetchw(&(lock->tail));
    ll_prefetchw(&(lock->flags));


    slot     = I_aepfl_lock_base_fetch_add_slot(&(lock->tail));
    flag_mem = I_aepfl_lock_base_get_next(lock->flags, slot);

    if (UNLIKELY(__atomic_load_n(flag_mem, __ATOMIC_RELAXED) ==
                 I_AEPFL_LOCKED)) {
#if I_WITH_AUTOLOCK
        struct kernel_autolock_abi * k_autolock_mem;
        k_autolock_mem = autolock_init_kernel_state();

        autolock_set_kernel_watch_mem(flag_mem, k_autolock_mem);
        autolock_set_kernel_watch_for(I_AEPFL_UNLOCKED, k_autolock_mem);
        autolock_set_kernel_watch_neq(0, k_autolock_mem);
#endif


        slot -= lock->head;
        if (slot <= I_AEPFL_PAUSE_BOUND) {
            lock_wait(
                LIKELY(__atomic_load_n(flag_mem, __ATOMIC_RELAXED) !=
                       I_AEPFL_LOCKED) /* recheck expression. */,
                ll_pause() /* wait expression. */, 0 /* wait iters. */,
                I_aepfl_has_lock /* target when succeeded. */);
        }
        else {
            lock_wait(
                LIKELY(__atomic_load_n(flag_mem, __ATOMIC_RELAXED) !=
                       I_AEPFL_LOCKED) /* recheck expression. */,
                ll_pause() /* wait expression. */,
                I_AEPFL_PAUSE_ITERS /* wait iters. */,
                I_aepfl_has_lock /* target when succeeded. */);
            lock_wait(
                LIKELY(__atomic_load_n(flag_mem, __ATOMIC_RELAXED) !=
                       I_AEPFL_LOCKED) /* recheck expression. */,
                yield() /* wait expression. */, 0 /* wait iters. */,
                I_aepfl_has_lock /* target when succeeded. */);
        }
    I_aepfl_has_lock:
        (void)(0);
#if I_WITH_AUTOLOCK
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
    }
    return I_SUCCESS;
}

extern_C_end();

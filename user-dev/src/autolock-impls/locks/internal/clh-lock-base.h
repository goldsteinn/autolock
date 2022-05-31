
/* NB: This file is meant to be included by another with I_WITH_AUTOLOCK
 * as zero or non-zero. */

#ifndef I_WITH_AUTOLOCK
#error "Never include clh-lock-base.h directly!"
#endif

/* CLH lock ported to use take advantage of kernel autolock feature.
 *
 * CLH Lock design summary:
 * The CLH lock is a FIFO lock that uses two pointers (head and tail of
 * the waiting list) Each thread has its context, composed of a memory
 * location on which any thread can spin, and a pointer to the context
 * to use for the next lock acquisition (a thread gives its context when
 * it releases the lock).
 * - On lock, the thread adds its current context to the tail of the
 * waiting list and spins on the memory space of the thread before it
 * - When the thread acquires the lock, it takes the context of the
 * thread that has unlocked him (the context shifts from thread to
 * thread)
 * - On unlock, the thread simply wakes the thread at the head of the
 * waiting list.
 *
 */


#include "arch/ll-prefetch.h"
#include "util/attrs.h"
#include "util/cpu-info.h"

extern_C_start();
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/locks/internal/lock-wait-common.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CLH_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CLH_LOCK_BASE_H_


/********************************************************************/
/* Lock constants. */
enum { I_CLH_UNLOCKED = 1, I_CLH_LOCKED = 0, I_CLH_PAUSE_ITERS = 512 };

/********************************************************************/
/* Internal node struct. */
typedef struct I_clh_node {
    union {
        uint32_t mem;
        uint8_t  I_padding0[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
} I_clh_node_t ALIGNED(L1_CACHE_LINE_SIZE);

/* Lock struct (exported). */
typedef struct I_clh_lock_base {
    union {
        I_clh_node_t * tail;
        uint8_t        I_padding0[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
    union {
        I_clh_node_t * head;
        uint8_t        I_padding1[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
} I_clh_lock_base_t ALIGNED(L1_CACHE_LINE_SIZE);


/********************************************************************/
/* Internal node functions. */
static MALLOC_FUNC I_clh_node_t * I_clh_lock_base_node_new(
    uint32_t lock_status);
static NONNULL(1) uint32_t
    I_clh_lock_base_node_free(I_clh_node_t * node);


/********************************************************************/
/* clh lock base api. */
static NONNULL(1) int32_t
    I_clh_lock_base_destroy(I_clh_lock_base_t * lock);
static NONNULL(1) int32_t
    I_clh_lock_base_trylock(I_clh_lock_base_t * lock);
/* lock api below. */
static NONNULL(1) int32_t
    I_clh_lock_base_unlock(I_clh_lock_base_t * lock);


/********************************************************************/
/* API implementation. */

static int32_t
I_clh_lock_base_destroy(I_clh_lock_base_t * lock) {
    I_clh_lock_base_node_free(lock->tail);
    return I_SUCCESS;
}


static int32_t
I_clh_lock_base_trylock(I_clh_lock_base_t * lock) {
    /* No real way to do trylock. */
    (void)(lock);
    return I_FAILURE;
}


static int32_t
I_clh_lock_base_unlock(I_clh_lock_base_t * lock) {
    __atomic_store_n(&(lock->head->mem), I_CLH_UNLOCKED,
                     __ATOMIC_RELAXED);

    return I_SUCCESS;
}


/********************************************************************/
/* Internal function implementation. */


static uint32_t
I_clh_lock_base_node_free(I_clh_node_t * node) {
    free(node);
    return I_SUCCESS;
}


static I_clh_node_t *
I_clh_lock_base_node_new(uint32_t lock_status) {
    I_clh_node_t * node = (I_clh_node_t *)aligned_alloc(
        L1_CACHE_LINE_SIZE, sizeof(I_clh_node_t));
    node->mem = lock_status;
    return node;
}
#endif

static NONNULL(1) int32_t
    CAT(I_clh_lock_base_init,
        I_WITH_AUTOLOCK)(I_clh_lock_base_t * lock);

static NONNULL(1) int32_t
    CAT(I_clh_lock_base_lock,
        I_WITH_AUTOLOCK)(I_clh_lock_base_t * lock);

static int32_t
CAT(I_clh_lock_base_init, I_WITH_AUTOLOCK)(I_clh_lock_base_t * lock) {
    /* Allocate here so we can unconditionally free. This speeds up lock
     * code. */
    lock->tail = I_clh_lock_base_node_new(I_CLH_UNLOCKED);
    lock->head = NULL;
#if I_WITH_AUTOLOCK
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    asm volatile("" : : :);
#endif
    return I_SUCCESS;
}


static int32_t
CAT(I_clh_lock_base_lock, I_WITH_AUTOLOCK)(I_clh_lock_base_t * lock) {
    I_clh_node_t *node, *prev_node;
    ll_prefetchw(lock);
    node = I_clh_lock_base_node_new(I_CLH_LOCKED);
    prev_node =
        __atomic_exchange_n(&(lock->tail), node, __ATOMIC_RELAXED);


    if (UNLIKELY(__atomic_load_n(&(prev_node->mem), __ATOMIC_RELAXED) ==
                 I_CLH_LOCKED)) {

#if I_WITH_AUTOLOCK
        struct kernel_autolock_abi * k_autolock_mem;
        k_autolock_mem = autolock_init_kernel_state();

        autolock_set_kernel_watch_for(I_CLH_UNLOCKED, k_autolock_mem);
        autolock_set_kernel_watch_neq(0, k_autolock_mem);
        autolock_set_kernel_watch_mem(&(prev_node->mem),
                                      k_autolock_mem);

#endif

        if (lock->head == prev_node) {
            /* we are next in queue. Don't yield. */
            lock_wait(LIKELY(__atomic_load_n(&(prev_node->mem),
                                             __ATOMIC_RELAXED) !=
                             I_CLH_LOCKED) /* recheck expression. */,
                      ll_pause() /* wait expression. */,
                      0 /* pause loop forever. */,
                      I_clh_has_lock /* target when succeeded. */);
        }
        else {
            //            yield();
            lock_wait(LIKELY(__atomic_load_n(&(prev_node->mem),
                                             __ATOMIC_RELAXED) !=
                             I_CLH_LOCKED) /* recheck expression. */,
                      ll_pause() /* wait expression. */,
                      I_CLH_PAUSE_ITERS /* wait iters. */,
                      I_clh_has_lock /* target when succeeded. */);

#if 0
            futex_wait(&(prev_node->mem), I_CLH_UNLOCKED);
#else
            lock_wait(LIKELY(__atomic_load_n(&(prev_node->mem),
                                             __ATOMIC_RELAXED) !=
                             I_CLH_LOCKED) /* recheck expression. */,
                      yield() /* wait expression. */,
                      0 /* yield loop forever. */,
                      I_clh_has_lock /* target when succeeded. */);
#endif
        }
    I_clh_has_lock:
        (void)(0);
#if I_WITH_AUTOLOCK
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
    }
    I_clh_lock_base_node_free(prev_node);


    /* Save info for unlock. */
    lock->head = node;
    return I_SUCCESS;
}


extern_C_end();


/* Non-compatible API. Probably going to throw-away. */

/* NB: This file is meant to be included by another with I_WITH_AUTOLOCK
 * as zero or non-zero. */

#ifndef I_WITH_AUTOLOCK
#error "Never include clh-lock-base.h directly!"
#endif

/*
 * John M. Mellor-Crummey and Michael L. Scott. 1991.
 * Algorithms for scalable synchronization on shared-memory
 * multiprocessors. ACM Trans. Comput. Syst. 9, 1 (February 1991).
 *
 * Lock design summary:
 * The MCS lock is one of the most known multicore locks.
 * Its main goal is to avoid all threads spinning on the same memory
 * address as it induces contention due to the cache coherency protocol.
 * The lock is organized as a FIFO list: this ensures total fairness.
 * Each thread as its own context, which is a node that the thread will
 * put into the linked list (representing the list of threads waiting
 * for the lock) when it tries to grab the lock. The lock is a
 * linked-list composed of a pointer to the tail of the list.
 * - On lock: the thread does an atomic xchg to put itself to the end of
 * the linked list and get the previous tail of the list. If there was
 * no other thread waiting, then the thread has the lock. Otherwise, the
 * thread spins on a memory address contained in its context.
 * - On unlock: if there is any thread, we just wake the next thread on
 * the waiting list. Otherwise we set the tail of the queue to NULL.
 */

#include "arch/ll-prefetch.h"
#include "util/attrs.h"

extern_C_start();
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/locks/internal/lock-wait-common.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__MCS_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__MCS_LOCK_BASE_H_

/********************************************************************/
/* Lock constants. */
enum { I_MCS_LOCKED = 1, I_MCS_UNLOCKED = 0, I_MCS_PAUSE_ITERS = 128 };


/********************************************************************/
/* Internal node struct. */
typedef struct I_mcs_node {
    uint32_t            mem;
    struct I_mcs_node * next;
} I_mcs_node_t;

/* Lock struct (exported). */
typedef struct I_mcs_lock_base {
    I_mcs_node_t * tail;
} I_mcs_lock_base_t;


/********************************************************************/
/* Internal node functions. */
static NONNULL(1) int32_t
    I_mcs_lock_base_node_init(I_mcs_node_t * node,
                              uint32_t       init_mem,
                              I_mcs_node_t * init_next);

/********************************************************************/
/* mcs lock base api. */

static NONNULL(1) uint32_t
    I_mcs_lock_base_destroy(I_mcs_lock_base_t * lock);
static NONNULL(1, 2) uint32_t
    I_mcs_lock_base_trylock(I_mcs_lock_base_t * lock,
                            I_mcs_node_t *      node);
/* lock api below. */
static NONNULL(1, 2) uint32_t
    I_mcs_lock_base_unlock(I_mcs_lock_base_t * lock,
                           I_mcs_node_t *      node);

/********************************************************************/
/* API implementation. */


static uint32_t
I_mcs_lock_base_destroy(I_mcs_lock_base_t * lock) {
    (void)(lock);
    return I_SUCCESS;
}

static uint32_t
I_mcs_lock_base_trylock(I_mcs_lock_base_t * lock, I_mcs_node_t * node) {
    I_mcs_node_t * expec;
    if (__atomic_load_n(&(lock->tail), __ATOMIC_RELAXED) != NULL) {
        return I_FAILURE;
    }

    I_mcs_lock_base_node_init(node, I_MCS_UNLOCKED, NULL);

    expec = NULL;
    if (LIKELY(__atomic_compare_exchange_n(
            &(lock->tail), &expec, node, true /* weak. */,
            __ATOMIC_RELAXED, __ATOMIC_RELAXED))) {
        return I_SUCCESS;
    }

    return I_FAILURE;
}


static uint32_t
I_mcs_lock_base_unlock(I_mcs_lock_base_t * lock, I_mcs_node_t * node) {
    I_mcs_node_t * next;
    if (node->next == NULL) {
        I_mcs_node_t * expec = node;
        if (LIKELY(__atomic_compare_exchange_n(
                &(lock->tail), &expec, NULL, false /* strong. */,
                __ATOMIC_RELAXED, __ATOMIC_RELAXED))) {
            return I_SUCCESS;
        }
        /* Only a few iterations expected.  */
        while ((next = __atomic_load_n(&(node->next),
                                       __ATOMIC_RELAXED)) == NULL) {
            ll_pause();
        }
    }


    futex_wake(&(next->mem), 1 /* Wake 1. */);
}

/********************************************************************/
/* Internal implementation. */
static int32_t
I_mcs_lock_base_node_init(I_mcs_node_t * node,
                          uint32_t       init_mem,
                          I_mcs_node_t * init_next) {
    node->mem  = init_mem;
    node->next = init_next;
    return I_SUCCESS;
}

#endif

static NONNULL(1) uint32_t
    CAT(I_mcs_lock_base_init,
        I_WITH_AUTOLOCK)(I_mcs_lock_base_t * lock);

static NONNULL(1, 2) uint32_t
    CAT(I_mcs_lock_base_lock, I_WITH_AUTOLOCK)(I_mcs_lock_base_t * lock,
                                               I_mcs_node_t * node);


static uint32_t
CAT(I_mcs_lock_base_init, I_WITH_AUTOLOCK)(I_mcs_lock_base_t * lock) {
    lock->tail = NULL;
#if I_WITH_AUTOLOCK
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    asm volatile("" : : :);
#endif
    return I_SUCCESS;
}

static uint32_t
CAT(I_mcs_lock_base_lock, I_WITH_AUTOLOCK)(I_mcs_lock_base_t * lock,
                                           I_mcs_node_t *      node) {
    I_mcs_node_t * tail;

    ll_prefetchw(lock);
    I_mcs_lock_base_node_init(node, I_MCS_UNLOCKED, NULL);
    tail = __atomic_exchange_n(&(lock->tail), node, __ATOMIC_RELAXED);

    /* We own the lock. */
    if (UNLIKELY(tail != NULL)) {
#if I_WITH_AUTOLOCK
        struct kernel_autolock_abi * k_autolock_mem;
        k_autolock_mem = autolock_init_kernel_state();

        autolock_set_kernel_watch_mem(&(node->mem), k_autolock_mem);
        autolock_set_kernel_watch_for(I_CLH_LOCKED, k_autolock_mem);
        autolock_set_kernel_watch_neq(0, k_autolock_mem);
#endif

        tail->next = node;

        lock_wait(
            LIKELY(__atomic_load_n(&(node->mem), __ATOMIC_RELAXED) !=
                   I_CLH_UNLOCKED) /* recheck expression. */,
            ll_pause() /* wait expression. */,
            I_MCS_PAUSE_ITERS /* wait iters. */,
            I_clh_has_lock /* target when succeeded. */);

        futex_wait(&(node->mem), I_MCS_UNLOCKED);
    I_clh_has_lock:
        (void)(0);
#if I_WITH_AUTOLOCK
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
    }

    return I_SUCCESS;
}

extern_C_end();

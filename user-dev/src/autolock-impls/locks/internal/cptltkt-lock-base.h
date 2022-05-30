
/*
 * David Dice, Virendra J. Marathe, and Nir Shavit. 2015.
 * Lock Cohorting: A General Technique for Designing NUMA Locks.
 * ACM Trans. Parallel Comput. 1, 2, Article 13 (February 2015).
 *
 * For a description of the algorithm, see cbomcs.c
 * The main difference between C-BO-MCS and C-PTL-TKT is that C-PTL-TKT
 * uses a Ticket lock for local locks and a Partitioned-Ticket lock for
 * the global lock.
 */

#include "arch/ll-prefetch.h"
#include "util/attrs.h"
#include "util/cpu-info.h"
#include "util/error-util.h"

extern_C_start();
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/locks/internal/lock-wait-common.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CPTLTKT_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CPTLTKT_LOCK_BASE_H_


/********************************************************************/
/* Lock constants. */
enum {
    I_CPTLTKT_HAS_SOCKET         = 1,
    I_CPTLTKT_DOESNT_HAVE_SOCKET = 0,

    I_CPTLTKT_SOCKET_BATCH_COUNT  = 100,
    I_CPTLTKT_SOCKET_NUM_SPINNERS = NUM_NUMA_NODES,

    I_CPTLTKT_PAUSE_ITERS = 512,
};

/********************************************************************/
/* Internal flag struct. */

typedef struct I_cptltkt_cpu_tkl {
    union {
        struct {
            union {
                struct {
                    uint32_t grant;
                    uint32_t requests;
                };

                uint64_t mem64;
            };
        };
        uint8_t padding0[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
    union {
        struct {
            uint32_t socket_grant;
            uint32_t batch_count;
            uint32_t owner_req_num;
        };
        uint8_t padding1[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);

} I_cptltkt_cpu_tkl_t ALIGNED(L1_CACHE_LINE_SIZE);

typedef union I_cptltkl_cache_line_u32 {
    union {
        uint32_t val32;
        uint8_t  padding0[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
} I_cptltkl_cache_line_u32_t ALIGNED(L1_CACHE_LINE_SIZE);

typedef struct I_cptltkt_socket_tkl {
    union {
        struct {
            union {
                uint32_t request;
                uint8_t  padding0[L1_CACHE_LINE_SIZE];
            } ALIGNED(L1_CACHE_LINE_SIZE);

            I_cptltkl_cache_line_u32_t
                grants[I_CPTLTKT_SOCKET_NUM_SPINNERS];
        } ALIGNED(L1_CACHE_LINE_SIZE);

        uint8_t padding1[L1_CACHE_LINE_SIZE];
    } ALIGNED(L1_CACHE_LINE_SIZE);
} I_cptltkt_socket_tkl_t ALIGNED(L1_CACHE_LINE_SIZE);


typedef struct I_cptltkt_lock_base {
    I_cptltkt_socket_tkl_t inter_socket_lock;
    I_cptltkt_cpu_tkl_t    per_socket_lock[NUM_NUMA_NODES];

    I_cptltkt_cpu_tkl_t * cur_psl;
} I_cptltkt_lock_base_t ALIGNED(L1_CACHE_LINE_SIZE);


/********************************************************************/
/* cptltkt lock base api. */
static NONNULL(1) int32_t
    I_cptltkt_lock_base_init(I_cptltkt_lock_base_t * lock);
static NONNULL(1) int32_t
    I_cptltkt_lock_base_destroy(I_cptltkt_lock_base_t * lock);
static NONNULL(1) int32_t
    I_cptltkt_lock_base_trylock(I_cptltkt_lock_base_t * lock);
/* lock api below. */
static NONNULL(1) int32_t
    I_cptltkt_lock_base_unlock(I_cptltkt_lock_base_t * lock);


/********************************************************************/
/* api implementation. */
static int32_t
I_cptltkt_lock_base_init(I_cptltkt_lock_base_t * lock) {
    __builtin_memset(lock, 0, sizeof(*lock));
#if I_WITH_AUTOLOCK
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    asm volatile("" : : :);
#endif
    return I_SUCCESS;
}

static int32_t
I_cptltkt_lock_base_destroy(I_cptltkt_lock_base_t * lock) {
    (void)(lock);
    return I_SUCCESS;
}

static int32_t
I_cptltkt_lock_base_trylock(I_cptltkt_lock_base_t * lock) {
    (void)(lock);
    assert(0);
}


static int32_t
I_cptltkt_lock_base_unlock(I_cptltkt_lock_base_t * lock) {
    uint32_t              next_grant, req_num, req_idx;
    I_cptltkt_cpu_tkl_t * local_lock = lock->cur_psl;

    next_grant =
        __atomic_load_n(&(local_lock->grant), __ATOMIC_RELAXED) + 1;
    if (LIKELY(__atomic_load_n(&(local_lock->requests),
                               __ATOMIC_RELAXED) != next_grant)) {
        if (local_lock->batch_count != 0) {
            --(local_lock->batch_count);
            __atomic_store_n(&(local_lock->socket_grant),
                             I_CPTLTKT_HAS_SOCKET, __ATOMIC_RELAXED);
            /* Cant reorder these. */
            asm volatile("" : : : "memory");
            __atomic_store_n(&(local_lock->grant), next_grant,
                             __ATOMIC_RELAXED);
            return I_SUCCESS;
        }
        local_lock->batch_count = I_CPTLTKT_SOCKET_BATCH_COUNT;
    }

    req_num = local_lock->owner_req_num + 1;
    req_idx = req_num % I_CPTLTKT_SOCKET_NUM_SPINNERS;

    /* These can safely reorder (generally better perf not too). */
    __atomic_store_n(&(lock->inter_socket_lock.grants[req_idx].val32),
                     req_num, __ATOMIC_RELAXED);
    __atomic_store_n(&(local_lock->grant), next_grant,
                     __ATOMIC_RELAXED);
    return I_SUCCESS;
}

/********************************************************************/
/* Internal API implementation. */


#endif

static NONNULL(1) int32_t
    CAT(I_cptltkt_lock_base_lock,
        I_WITH_AUTOLOCK)(I_cptltkt_lock_base_t * lock);


static int32_t
CAT(I_cptltkt_lock_base_lock,
    I_WITH_AUTOLOCK)(I_cptltkt_lock_base_t * lock) {
    uint32_t req_num, req_idx;

    I_cptltkt_cpu_tkl_t * local_lock =
        &(lock->per_socket_lock[get_current_socket()]);
    req_num = __atomic_fetch_add(&(local_lock->requests), 1,
                                 __ATOMIC_RELAXED);
    PRINTFFL;

#if I_WITH_AUTOLOCK
    struct kernel_autolock_abi * k_autolock_mem;
    k_autolock_mem = autolock_init_kernel_state();
    autolock_set_kernel_watch_for(req_num, k_autolock_mem);
    autolock_set_kernel_watch_neq(0, k_autolock_mem);
    autolock_set_kernel_watch_mem(&(local_lock->grant), k_autolock_mem);
#endif

    lock_wait(LIKELY(__atomic_load_n(&(local_lock->grant),
                                     __ATOMIC_RELAXED) ==
                     req_num) /* recheck expression. */,
              ll_pause() /* wait expression. */,
              I_CPTLTKT_PAUSE_ITERS /* wait iters. */,
              I_cptltkt_has_local_lock /* target when succeeded. */);


    lock_wait(LIKELY(__atomic_load_n(&(local_lock->grant),
                                     __ATOMIC_RELAXED) ==
                     req_num) /* recheck expression. */,
              yield() /* wait expression. */,
              0 /* yield loop forever. */,
              I_cptltkt_has_local_lock /* target when succeeded. */);


I_cptltkt_has_local_lock:

    /* Check if this socket already owns the inter-socket lock. */
    if (LIKELY(__atomic_load_n(&(local_lock->socket_grant),
                               __ATOMIC_RELAXED) !=
               I_CPTLTKT_DOESNT_HAVE_SOCKET)) {
        __atomic_store_n(&(local_lock->socket_grant),
                         I_CPTLTKT_DOESNT_HAVE_SOCKET,
                         __ATOMIC_RELAXED);
#if I_WITH_AUTOLOCK
        autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif
        return I_SUCCESS;
    }

    req_num = __atomic_fetch_add(&(lock->inter_socket_lock.request), 1,
                                 __ATOMIC_RELAXED);
    req_idx = req_num % I_CPTLTKT_SOCKET_NUM_SPINNERS;

#if I_WITH_AUTOLOCK
    autolock_set_kernel_watch_for(req_num, k_autolock_mem);
    autolock_set_kernel_watch_neq(0, k_autolock_mem);
    autolock_set_kernel_watch_mem(
        &(lock->inter_socket_lock.grants[req_idx].val32),
        k_autolock_mem);
#endif

    lock_wait(
        LIKELY(__atomic_load_n(
                   &(lock->inter_socket_lock.grants[req_idx].val32),
                   __ATOMIC_RELAXED) ==
               req_num) /* recheck expression. */,
        yield() /* wait expression. */, 0 /* wait iters. */,
        I_cptltkt_has_global_lock /* target when succeeded. */);

I_cptltkt_has_global_lock:

#if I_WITH_AUTOLOCK
    autolock_set_kernel_watch_mem(NULL, k_autolock_mem);
#endif

    lock->cur_psl             = local_lock;
    local_lock->owner_req_num = req_num;
    return I_SUCCESS;
}

extern_C_end();

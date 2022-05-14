#ifndef _SRC__LOCKS__LOCK_BASE_H_
#define _SRC__LOCKS__LOCK_BASE_H_

#include "util/sched-util.h"
#include "util/types.h"

#include "arch/ll-pause.h"
#include "arch/ll-timing.h"

/* Base memory for all lock configs. */
typedef struct lock_base {
    union {
        struct {
            uint32_t mem; /* memory to implement lock on. */
            uint32_t unused;
            uint32_t waiters; /* memory for managing futex waiters
                                 (possibly unused). */
        } state;
        uint8_t unused[16] __attribute__((aligned(16)));
    };
} lock_base_t;

template<
    uint32_t max_raw_iter,   /* Max iters till either yield or futex. */
    uint32_t max_yield_iter, /* Max iters in yield loop. */
    uint32_t with_backoff,   /* don't spin directly on lock. */
    uint32_t with_yield,     /* desched if fail to acquire   lock. */
    uint32_t with_futex /* Use futex if we fail for long enough. */>
class lock_ops {

    static constexpr uint32_t kUNLOCKED = 1;
    static constexpr uint32_t kLOCKED   = 0;

    static constexpr uint32_t kSUCCESS = 0;
    static constexpr uint32_t kFAILURE = 1;


    static constexpr uint32_t kmax_raw_iter   = max_raw_iter;
    static constexpr uint32_t kmax_yield_iter = max_yield_iter;

    static constexpr uint32_t kwith_backoff = with_backoff;
    static constexpr uint32_t kwith_yield   = with_yield;
    static constexpr uint32_t kwith_futex   = with_futex;


   private:
    static uint32_t
    backoff_init() {
        if constexpr (kwith_backoff) {
            /* Fast and pseudo random. Never want zero. */
            return get_ll_time() & 31;
        }
        else {
            return 0;
        }
    }

    static uint32_t
    backoff_next(uint32_t backoff) {
        if constexpr (kwith_backoff) {
            /* We just need to stagger the threads really. Too many
             * iters will start to negatively impact latency. */
            return (backoff + backoff + 1) & 511;
        }
        else {
            return 0;
        }
    }

    static void
    pause() {
        /* Just asm("pause"). Causes micro-arch to not aggressively
         * speculate ahead in lock loop. */
        ll_pause();
    }


   public:
    /* Init lock memory. */
    static NONNULL(1) int32_t init(lock_base_t * lock) {
        __builtin_memset(lock, 0, sizeof(*lock));
        lock->state.mem = kUNLOCKED;
        return kSUCCESS;
    }

    static NONNULL(1) int32_t destroy(lock_base_t * lock) {
        (void)(lock);
        return kSUCCESS;
    }

    /* Standard try lock. */
    static NONNULL(1) uint32_t try_lock(lock_base_t * lock) {
        if (LIKELY((__atomic_load_n(&(lock->state.mem),
                                    __ATOMIC_RELAXED)) != kLOCKED)) {
            if (LIKELY((__atomic_exchange_n(&(lock->state.mem), kLOCKED,
                                            __ATOMIC_RELAXED)) !=
                       kLOCKED)) {
                return kSUCCESS;
            }
        }
        return kFAILURE;
    }

    static NONNULL(1) int32_t lock(lock_base_t * lock) {
        uint32_t iter_count, i, backoff;

        /* Allow these to be unused as some of them are config
         * dependendent. */
        (void)(iter_count);
        (void)(i);
        (void)(backoff);

        for (;;) {
            /* Always try lock unconditionally first. */
            if (try_lock(lock) == kSUCCESS) {
                return kSUCCESS;
            }

            /* If we have max raw iters start a counter. */
            if constexpr (kmax_raw_iter) {
                iter_count = 0;
            }


            /* If we have backoff initialize after first failure. */
            if constexpr (kwith_backoff) {
                backoff = backoff_init();
            }

            for (;;) {
                if (LIKELY(try_lock(lock) == kSUCCESS)) {
                    return kSUCCESS;
                }

                /* Check if we reach max iter count. */
                if constexpr (kmax_raw_iter) {
                    if (UNLIKELY(++iter_count >= kmax_raw_iter)) {
                        break;
                    }
                }

                /* If we fail have have backoff spin for a bit before
                 * next attempt. */
                if constexpr (kwith_backoff) {
                    for (i = backoff; i; --i) {
                        pause();
                    }
                    backoff = backoff_next(backoff);
                }
                else {
                    pause();
                }
            }


            /* We broke from raw loop. Now do yield loop. */
            if constexpr (kwith_yield) {

                /* Initialize max iters conditionally. */
                if constexpr (kmax_yield_iter) {
                    iter_count = 0;
                }
                for (;;) {
                    if (LIKELY(try_lock(lock) == kSUCCESS)) {
                        return kSUCCESS;
                    }
                    if constexpr (kmax_yield_iter) {
                        if (++iter_count >= kmax_yield_iter) {
                            break;
                        }
                    }
                    /* Yield between attempts. */
                    yield();
                }
            }

            /* Enter futex. We have failed for long enough. */
            if constexpr (kwith_futex) {

                /* Counter so unlock knows if it needs to wake a thread.
                 */
                __atomic_fetch_add(&(lock->state.waiters), 1,
                                   __ATOMIC_RELAXED);

                /* We can silently ignore futex errors and still be
                 * correct. */
                futex_wait(&(lock->state.mem), kLOCKED);

                /* dec counter. Its safe to call futex_unlock with no
                 * threads waiting but ultimately adds a syscall to
                 * unlock overhead so best to avoid it if possible. */
                __atomic_fetch_sub(&(lock->state.waiters), 1,
                                   __ATOMIC_RELAXED);
            }
        }
    }

    static NONNULL(1) int32_t unlock(lock_base_t * lock) {
        /* Set lock unlocked. */
        __atomic_store_n(&(lock->state.mem), kUNLOCKED,
                         __ATOMIC_RELAXED);
        if constexpr (kwith_futex) {
            /* If its possible we have a waiter then futex unlock. We
             * must write unlock first otherwise a thread may hang on
             * the futex. Also note the unlock write and waiters read is
             * strongly ordered. x86_64 TSA does this for us but on
             * other architectures we would need some explicit
             * barrier. */
            if (__atomic_load_n(&(lock->state.waiters),
                                __ATOMIC_RELAXED)) {
                /* Futex error here could lead to deadlock so don't
                 * silently ignore. */
                if (UNLIKELY(futex_wake(&(lock->state.mem),
                                        1 /* wake one. */))) {
                    return kFAILURE;
                }
            }
        }
        return kSUCCESS;
    }
};

#endif

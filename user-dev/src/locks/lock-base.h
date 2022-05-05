#ifndef _SRC__LOCKS__LOCK_BASE_H_
#define _SRC__LOCKS__LOCK_BASE_H_

#include "util/sched-util.h"
#include "util/types.h"

#include "arch/ll-pause.h"
#include "arch/ll-timing.h"

typedef struct lock_base {
    union {
        struct {
            uint32_t mem;
            uint32_t unused;
            uint32_t waiters; /* Possibly unused. */
        } state;
        uint8_t unused[16] __attribute__((aligned(16)));
    };
} lock_base_t;

template<uint32_t max_raw_iter,
         uint32_t max_yield_iter,
         uint32_t with_backoff,
         uint32_t with_yield,
         uint32_t with_futex>
class lock_ops {
    static constexpr uint32_t kUNLOCKED = 0;
    static constexpr uint32_t kLOCKED   = 1;


    static constexpr uint32_t kmax_raw_iter   = max_raw_iter;
    static constexpr uint32_t kmax_yield_iter = max_yield_iter;

    static constexpr uint32_t kwith_backoff = with_backoff;
    static constexpr uint32_t kwith_yield   = with_yield;
    static constexpr uint32_t kwith_futex   = with_futex;


   private:
    static uint32_t
    backoff_init() {
        if constexpr (kwith_backoff) {
            return get_ll_time() & 31;
        }
        else {
            return 0;
        }
    }

    static uint32_t
    backoff_next(uint32_t backoff) {
        if constexpr (kwith_backoff) {
            return (backoff + backoff) & 512;
        }
        else {
            return 0;
        }
    }

    static void
    pause() {
        ll_pause();
    }


   public:
    static void
    init(lock_base_t * lock) {
        lock->state.mem = kUNLOCKED;
        __builtin_memset(lock, 0, sizeof(*lock));
    }

    static uint32_t
    try_lock(lock_base_t * lock) {
        uint32_t ret;
        if (LIKELY((ret = __atomic_load_n(&(lock->state.mem),
                                          __ATOMIC_RELAXED)) == kUNLOCKED)) {
            if (LIKELY((ret = __atomic_exchange_n(&(lock->state.mem), kLOCKED,
                                                  __ATOMIC_RELAXED)) ==
                       kUNLOCKED)) {
                return kUNLOCKED;
            }
        }
        die_assert(ret != kUNLOCKED);
        return ret;
    }

    static void
    lock(lock_base_t * lock) {
        uint32_t iter_count, i, backoff;

        for (;;) {
            if (try_lock(lock) == kUNLOCKED) {
                return;
            }

            if constexpr (kmax_raw_iter) {
                iter_count = 0;
            }

            if constexpr (kwith_backoff) {
                backoff = backoff_init();
            }

            for (;;) {
                if (LIKELY(try_lock(lock) == kUNLOCKED)) {
                    return;
                }
                if constexpr (kmax_raw_iter) {
                    if (UNLIKELY(++iter_count >= kmax_raw_iter)) {
                        break;
                    }
                }

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

            if constexpr (kwith_yield) {
                if constexpr (kmax_yield_iter) {
                    iter_count = 0;
                }
                for (;;) {
                    if (LIKELY(try_lock(lock) == kUNLOCKED)) {
                        return;
                    }
                    if constexpr (kmax_yield_iter) {
                        if (++iter_count >= kmax_yield_iter) {
                            break;
                        }
                    }
                    yield();
                }
            }

            if constexpr (kwith_futex) {
                __atomic_fetch_add(&(lock->state.waiters), 1, __ATOMIC_RELAXED);
                futex_wait(&(lock->state.mem), kLOCKED);
                __atomic_fetch_sub(&(lock->state.waiters), 1, __ATOMIC_RELAXED);
            }
        }
    }

    static void
    unlock(lock_base_t * lock) {
        __atomic_store_n(&(lock->state.mem), kUNLOCKED, __ATOMIC_RELAXED);
        if constexpr (kwith_futex) {

            if (__atomic_load_n(&(lock->state.waiters), __ATOMIC_RELAXED)) {
                futex_wake(&(lock->state.mem), 1);
            }
        }
    }
};

#endif

#include <pthread.h>

#include "arch/ll-pause.h"
#include "interpose/pthread-mutex-interpose.h"
#include "util/attrs.h"
#include "util/types.h"

typedef pthread_spinlock_t I_pthread_spin_t;


#define I_pthread_spin_init(spin) pthread_spin_init(spin, 0)
#define I_pthread_spin_destroy    pthread_spin_destroy

/* We have these special functions because many people use
 * PTHREAD_MUTEX_INITIALIZER (zeroing memory) and older version of
 * pthread_spin_* are not compatible with that initialization. This is
 * not really meant to be safe or used in benchmarks. Moreso for a proof
 * of concept of overriding the pthread_mutex_* symbols in a way that
 * works with pthread_cond_*. */
#define I_pthread_spin_trylock(spin)                                   \
    ({                                                                 \
        uint32_t I_is_init = *(safe_uint32_t volatile *)((spin) + 1);  \
        if (UNLIKELY(I_is_init == 0)) {                                \
            if (__atomic_exchange_n(                                   \
                    (safe_uint32_t volatile *)((spin) + 2), 1,         \
                    __ATOMIC_RELAXED) == 0) {                          \
                pthread_spin_init(spin, 0);                            \
                *(safe_uint32_t volatile *)((spin) + 1) = 1;           \
                *(safe_uint32_t volatile *)((spin) + 2) = 0;           \
            }                                                          \
            else {                                                     \
                while (__atomic_load_n(                                \
                    (safe_uint32_t volatile *)((spin) + 2),            \
                    __ATOMIC_RELAXED)) {                               \
                    ll_pause();                                        \
                }                                                      \
            }                                                          \
        }                                                              \
                                                                       \
        pthread_spin_trylock(spin);                                    \
    })

#define I_pthread_spin_lock(spin)                                      \
    ({                                                                 \
        uint32_t I_is_init = *(safe_uint32_t volatile *)((spin) + 1);  \
        if (UNLIKELY(I_is_init == 0)) {                                \
            if (__atomic_exchange_n(                                   \
                    (safe_uint32_t volatile *)((spin) + 2), 1,         \
                    __ATOMIC_RELAXED) == 0) {                          \
                pthread_spin_init(spin, 0);                            \
                *(safe_uint32_t volatile *)((spin) + 1) = 1;           \
                *(safe_uint32_t volatile *)((spin) + 2) = 0;           \
            }                                                          \
            else {                                                     \
                while (__atomic_load_n(                                \
                    (safe_uint32_t volatile *)((spin) + 2),            \
                    __ATOMIC_RELAXED)) {                               \
                    ll_pause();                                        \
                }                                                      \
            }                                                          \
        }                                                              \
        pthread_spin_lock(spin);                                       \
    })

#define I_pthread_spin_unlock pthread_spin_unlock

create_pthread_interpose(I_pthread_spin);

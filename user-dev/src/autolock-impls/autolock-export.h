#ifndef _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_
#define _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_

#include "autolock-impls/internal/autolock-macro-helpers.h"

/* Custom autolocks. More so for reference than to use. */
#include "autolock-impls/locks/custom-backoff-autolock.h"
#include "autolock-impls/locks/custom-spin-autolock.h"
#include "autolock-impls/locks/rseq-autolock.h"


/* Comparison spin locks. */
#include "autolock-impls/locks/spin-comparisons.h"

/* Simple ticket lock (bad). */
#include "autolock-impls/locks/simple-ticket-autolock.h"
#include "autolock-impls/locks/simple-ticket-normlock.h"


/* Third party locks from papers. */
#include "autolock-impls/locks/aepfl-autolock.h"
#include "autolock-impls/locks/aepfl-normlock.h"
#include "autolock-impls/locks/clh-autolock.h"
#include "autolock-impls/locks/clh-normlock.h"
#include "autolock-impls/locks/cptltkt-autolock.h"
#include "autolock-impls/locks/cptltkt-normlock.h"
#include "autolock-impls/locks/mcs-autolock.h"
#include "autolock-impls/locks/mcs-normlock.h"

/* Add any implementations here. The capitol ones are generally macros
 * that can optionally be empty or the name of a lock. */
#define I_AUTOLOCK_IMPLS                                               \
    custom_spin_autolock, custom_backoff_autolock, clh_normlock,       \
        clh_autolock, RSEQ_AUTOLOCK, SIMPLE_TICKET_AUTOLOCK,           \
        SIMPLE_TICKET_NORMLOCK, MCS_AUTOLOCK, MCS_NORMLOCK,            \
        aepfl_autolock, aepfl_normlock, cptltkt_normlock,              \
        cptltkt_autolock,


#define I_gen_autolock(autolock_name)                                  \
    class autolock_name {                                              \
        using base_autolock = CAT(autolock_name, _t);                  \
                                                                       \
        base_autolock lock_internal;                                   \
                                                                       \
       public:                                                         \
        static autolock_name *                                         \
        init(void * init_mem) {                                        \
            autolock_name * lock = CAST(autolock_name *, init_mem);    \
            CAT(autolock_name, _init)(&(lock->lock_internal));         \
            return lock;                                               \
        }                                                              \
        int32_t                                                        \
        try_lock() {                                                   \
            return CAT(autolock_name, _trylock)(&lock_internal);       \
        }                                                              \
        void                                                           \
        lock() {                                                       \
            CAT(autolock_name, _lock)(&lock_internal);                 \
        }                                                              \
        void                                                           \
        unlock() {                                                     \
            CAT(autolock_name, _unlock)(&lock_internal);               \
        }                                                              \
        void                                                           \
        destroy() {                                                    \
            CAT(autolock_name, _destroy)(&lock_internal);              \
        }                                                              \
    } __attribute__((may_alias));


#define AUTOLOCK_IMPLS list_autolocks(I_AUTOLOCK_IMPLS), I_LOCK_COMPARISONS

gen_autolocks(I_gen_autolock, AUTOLOCK_IMPLS);

#endif

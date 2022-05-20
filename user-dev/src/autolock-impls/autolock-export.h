#ifndef _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_
#define _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_

#include "autolock-impls/internal/autolock-macro-helpers.h"

#include "autolock-impls/locks/backoff-autolock.h"
#include "autolock-impls/locks/rseq-autolock.h"
#include "autolock-impls/locks/simple-autolock.h"
#include "autolock-impls/locks/ticket-autolock.h"

#include "autolock-impls/locks/aepfl-autolock.h"
#include "autolock-impls/locks/aepfl-normlock.h"
#include "autolock-impls/locks/clh-autolock.h"
#include "autolock-impls/locks/clh-normlock.h"
#include "autolock-impls/locks/mcs-autolock.h"
#include "autolock-impls/locks/mcs-normlock.h"

/* Add any implementations here. The capitol ones are generally macros
 * that can optionally be empty or the name of a lock. */
#define I_AUTOLOCK_IMPLS                                               \
    simple_autolock, backoff_autolock, clh_autolock, clh_normlock,     \
        RSEQ_AUTOLOCK, TICKET_AUTOLOCK, MCS_AUTOLOCK, MCS_NORMLOCK,    \
        aepfl_autolock, aepfl_normlock


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


#define AUTOLOCK_IMPLS list_autolocks(I_AUTOLOCK_IMPLS)

gen_autolocks(I_gen_autolock, I_AUTOLOCK_IMPLS);

#endif

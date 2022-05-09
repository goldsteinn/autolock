#ifndef _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_
#define _SRC__AUTLOCK_IMPLS__AUTOLOCK_EXPORT_H_

#include "autolock-impls/simple-autolock.h"
#include "autolock-impls/backoff-autolock.h"

#define I_gen_autolock(name, autolock_name)                            \
    class name {                                                       \
        using base_autolock = CAT(autolock_name, _t);                  \
                                                                       \
        base_autolock lock_internal;                                   \
                                                                       \
       public:                                                         \
        static name *                                                  \
        init(void * init_mem) {                                        \
            name * lock = CAST(name *, init_mem);                      \
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


I_gen_autolock(auto_spinlock, simple_autolock);
I_gen_autolock(auto_backoff_lock, backoff_autolock);


#define AUTOLOCK_IMPLS auto_spinlock, auto_backoff_lock

#endif

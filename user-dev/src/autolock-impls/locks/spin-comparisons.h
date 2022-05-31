#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__SPIN_COMPARISONS_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__SPIN_COMPARISONS_H_

#include "util/macro.h"

#define I_decl_new_lock(name)                                          \
    typedef LOCK_BASE_MAKE_NAME(I_user_lock_base_t) CAT(name, _t);     \
    static typedef_func(CAT(name, _init),                              \
                        LOCK_BASE_MAKE_NAME(I_user_lock_base_init));   \
    static typedef_func(                                               \
        CAT(name, _destroy),                                           \
        LOCK_BASE_MAKE_NAME(I_user_lock_base_destroy));                \
    static typedef_func(                                               \
        CAT(name, _trylock),                                           \
        LOCK_BASE_MAKE_NAME(I_user_lock_base_trylock));                \
    static typedef_func(CAT(name, _unlock),                            \
                        LOCK_BASE_MAKE_NAME(I_user_lock_base_unlock)); \
    static typedef_func(CAT(name, _lock),                              \
                        LOCK_BASE_MAKE_NAME(I_user_lock_base_lock));


/* Configs. */
#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(spin_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(spin_normlock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(yield_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(yield_normlock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(backoff_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(backoff_normlock);

#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(backoff_yield_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    0

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(backoff_yield_normlock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(backoff_yield_futex_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(backoff_yield_futex_normlock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(backoff_futex_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  1
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(backoff_futex_normlock);

#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(futex_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    0
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(futex_normlock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 1
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"
I_decl_new_lock(yield_futex_autolock);


#undef I_WITH_AUTOLOCK
#undef I_WITH_YIELD
#undef I_WITH_BACKOFF
#undef I_WITH_FUTEX
#define I_WITH_AUTOLOCK 0
#define I_WITH_BACKOFF  0
#define I_WITH_YIELD    1
#define I_WITH_FUTEX    1

#include "autolock-impls/locks/internal/lock-base.h"

I_decl_new_lock(yield_futex_normlock);


#define I_LOCK_COMPARISONS                                             \
    spin_autolock, spin_normlock, yield_autolock, yield_normlock,      \
        backoff_autolock, backoff_normlock, backoff_yield_autolock,    \
        backoff_yield_normlock, backoff_yield_futex_autolock,          \
        backoff_yield_futex_normlock, backoff_futex_autolock,          \
        backoff_futex_normlock, futex_autolock, futex_normlock,        \
        yield_futex_autolock, yield_futex_normlock

#endif

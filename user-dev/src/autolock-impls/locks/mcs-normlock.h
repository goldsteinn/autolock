#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__MCS_NORMLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__MCS_NORMLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 0
#include "internal/mcs-lock-base.h"


typedef I_mcs_lock_base mcs_normlock_t;

static typedef_func(mcs_normlock_init, I_mcs_lock_base_init);
static typedef_func(mcs_normlock_destroy, I_mcs_lock_base_destroy);
static typedef_func(mcs_normlock_trylock, I_mcs_lock_base_trylock);
static typedef_func(mcs_normlock_lock, I_mcs_lock_base_lock0);
static typedef_func(mcs_normlock_unlock, I_mcs_lock_base_unlock);

/* Leave empty because api incompatible. */
#define MCS_NORMLOCK

#endif

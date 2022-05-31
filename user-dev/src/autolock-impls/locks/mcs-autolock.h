#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__MCS_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__MCS_AUTOLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 1
#include "internal/mcs-lock-base.h"


typedef I_mcs_lock_base mcs_autolock_t;

static typedef_func(mcs_autolock_init, I_mcs_lock_base_init1);
static typedef_func(mcs_autolock_destroy, I_mcs_lock_base_destroy);
static typedef_func(mcs_autolock_trylock, I_mcs_lock_base_trylock);
static typedef_func(mcs_autolock_lock, I_mcs_lock_base_lock1);
static typedef_func(mcs_autolock_unlock, I_mcs_lock_base_unlock);

/* Leave empty because api incompatible. */
#define MCS_AUTOLOCK

#endif

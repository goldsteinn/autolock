#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__AEPFL_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__AEPFL_AUTOLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 1
#include "internal/aepfl-lock-base.h"


typedef I_aepfl_lock_base aepfl_autolock_t;

static typedef_func(aepfl_autolock_init, I_aepfl_lock_base_init1);
static typedef_func(aepfl_autolock_destroy, I_aepfl_lock_base_destroy);
static typedef_func(aepfl_autolock_trylock, I_aepfl_lock_base_trylock);
static typedef_func(aepfl_autolock_lock, I_aepfl_lock_base_lock1);
static typedef_func(aepfl_autolock_unlock, I_aepfl_lock_base_unlock);


#endif

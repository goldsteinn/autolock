#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__AEPFL_NORMLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__AEPFL_NORMLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 0
#include "internal/aepfl-lock-base.h"


typedef I_aepfl_lock_base aepfl_normlock_t;

static typedef_func(aepfl_normlock_init, I_aepfl_lock_base_init0);
static typedef_func(aepfl_normlock_destroy, I_aepfl_lock_base_destroy);
static typedef_func(aepfl_normlock_trylock, I_aepfl_lock_base_trylock);
static typedef_func(aepfl_normlock_lock, I_aepfl_lock_base_lock0);
static typedef_func(aepfl_normlock_unlock, I_aepfl_lock_base_unlock);


#endif

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__CLH_NORMLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__CLH_NORMLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 0
#include "internal/clh-lock-base.h"


typedef I_clh_lock_base clh_normlock_t;

static typedef_func(clh_normlock_init, I_clh_lock_base_init0);
static typedef_func(clh_normlock_destroy, I_clh_lock_base_destroy);
static typedef_func(clh_normlock_trylock, I_clh_lock_base_trylock);
static typedef_func(clh_normlock_lock, I_clh_lock_base_lock0);
static typedef_func(clh_normlock_unlock, I_clh_lock_base_unlock);


#endif

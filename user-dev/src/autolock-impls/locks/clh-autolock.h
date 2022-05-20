#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__CLH_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__CLH_AUTOLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 1
#include "internal/clh-lock-base.h"


typedef I_clh_lock_base clh_autolock_t;

static typedef_func(clh_autolock_init, I_clh_lock_base_init);
static typedef_func(clh_autolock_destroy, I_clh_lock_base_destroy);
static typedef_func(clh_autolock_trylock, I_clh_lock_base_trylock);
static typedef_func(clh_autolock_lock, I_clh_lock_base_lock1);
static typedef_func(clh_autolock_unlock, I_clh_lock_base_unlock);


#endif

#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__CPTLTKT_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__CPTLTKT_AUTOLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 1
#include "internal/cptltkt-lock-base.h"


typedef I_cptltkt_lock_base cptltkt_autolock_t;

static typedef_func(cptltkt_autolock_init, I_cptltkt_lock_base_init);
static typedef_func(cptltkt_autolock_destroy, I_cptltkt_lock_base_destroy);
static typedef_func(cptltkt_autolock_trylock, I_cptltkt_lock_base_trylock);
static typedef_func(cptltkt_autolock_lock, I_cptltkt_lock_base_lock1);
static typedef_func(cptltkt_autolock_unlock, I_cptltkt_lock_base_unlock);


#endif

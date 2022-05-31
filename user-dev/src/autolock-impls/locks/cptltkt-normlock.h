#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__CPTLTKT_NORMLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__CPTLTKT_NORMLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 0
#include "internal/cptltkt-lock-base.h"


typedef I_cptltkt_lock_base cptltkt_normlock_t;

static typedef_func(cptltkt_normlock_init, I_cptltkt_lock_base_init0);
static typedef_func(cptltkt_normlock_destroy, I_cptltkt_lock_base_destroy);
static typedef_func(cptltkt_normlock_trylock, I_cptltkt_lock_base_trylock);
static typedef_func(cptltkt_normlock_lock, I_cptltkt_lock_base_lock0);
static typedef_func(cptltkt_normlock_unlock, I_cptltkt_lock_base_unlock);


#endif

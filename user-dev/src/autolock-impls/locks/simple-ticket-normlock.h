#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__SIMPLE_TICKET_NORMLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__SIMPLE_TICKET_NORMLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 0
#include "internal/simple-ticket-lock-base.h"


typedef I_simple_ticket_lock_base simple_ticket_normlock_t;

static typedef_func(simple_ticket_normlock_init,
                    I_simple_ticket_lock_base_init0);
static typedef_func(simple_ticket_normlock_destroy,
                    I_simple_ticket_lock_base_destroy);
static typedef_func(simple_ticket_normlock_trylock,
                    I_simple_ticket_lock_base_trylock);
static typedef_func(simple_ticket_normlock_lock,
                    I_simple_ticket_lock_base_lock0);
static typedef_func(simple_ticket_normlock_unlock,
                    I_simple_ticket_lock_base_unlock);

#define SIMPLE_TICKET_NORMLOCK


#endif

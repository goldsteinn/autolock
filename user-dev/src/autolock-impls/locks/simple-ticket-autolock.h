#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__SIMPLE_TICKET_AUTOLOCK_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__SIMPLE_TICKET_AUTOLOCK_H_

#undef I_WITH_AUTOLOCK
#define I_WITH_AUTOLOCK 1
#include "internal/simple-ticket-lock-base.h"


typedef I_simple_ticket_lock_base simple_ticket_autolock_t;

static typedef_func(simple_ticket_autolock_init,
                    I_simple_ticket_lock_base_init1);
static typedef_func(simple_ticket_autolock_destroy,
                    I_simple_ticket_lock_base_destroy);
static typedef_func(simple_ticket_autolock_trylock,
                    I_simple_ticket_lock_base_trylock);
static typedef_func(simple_ticket_autolock_lock,
                    I_simple_ticket_lock_base_lock1);
static typedef_func(simple_ticket_autolock_unlock,
                    I_simple_ticket_lock_base_unlock);

#define SIMPLE_TICKET_AUTOLOCK


#endif

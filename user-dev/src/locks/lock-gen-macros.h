#ifndef _SRC__LOCKS__LOCK_GEN_MACROS_H_
#define _SRC__LOCKS__LOCK_GEN_MACROS_H_

#include "util/macro.h"


/* non-empty case. */
#define I_list_add_lock_0(x) , x

/* empty case. */
#define I_list_add_lock_1(...)

/* x may be empty. */


#define I_list_add_lock(x) CAT(I_list_add_lock_, IS_EMPTY(x))(x)


#define list_locks(...)                                            \
    REMOVE_FIRST(APPLY_TOO(I_list_add_lock, EMPTY(), __VA_ARGS__))


#endif

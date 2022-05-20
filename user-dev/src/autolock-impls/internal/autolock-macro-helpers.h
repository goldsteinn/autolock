#ifndef _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_MACRO_HELPERS_H_
#define _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_MACRO_HELPERS_H_

#include "util/macro.h"

/* non-empty case. */
#define I_opt_gen_autolock_0(macro, x) macro(x)

/* empty case. */
#define I_opt_gen_autolock_1(...)


/* First unpack. */
#define I_opt_gen_autolock1(macro, ...)                                \
    CAT(I_opt_gen_autolock_, IS_EMPTY(__VA_ARGS__))(macro, __VA_ARGS__)


#define I_opt_gen_autolock0(...) I_opt_gen_autolock1(__VA_ARGS__)


#define I_opt_gen_autolock(...)                                        \
    I_opt_gen_autolock0(DEPAREN(__VA_ARGS__))


/* non-empty case. */
#define I_list_add_autolock_0(x) , x

/* empty case. */
#define I_list_add_autolock_1(...)

/* x may be empty. */


#define I_list_add_autolock(x) CAT(I_list_add_autolock_, IS_EMPTY(x))(x)


#define list_autolocks(...)                                            \
    REMOVE_FIRST(APPLY_TOO(I_list_add_autolock, EMPTY(), __VA_ARGS__))


#define gen_autolocks(gen_macro, ...)                                  \
    APPLY(I_opt_gen_autolock, ;, APPLY_PACKL(gen_macro, __VA_ARGS__))

#endif

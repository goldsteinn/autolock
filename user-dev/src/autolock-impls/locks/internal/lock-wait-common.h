#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__LOCK_WAIT_COMMON_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__LOCK_WAIT_COMMON_H_


#include "util/common.h"

#define lock_wait(recheck, wait_expr, wait_iters, done)                \
    {                                                                  \
        ensure_const_eval(wait_iters);                                 \
        ensure_const_eval(-(wait_iters));                              \
                                                                       \
        if (choose_const_expr((wait_iters), 1, 0)) {                   \
            uint32_t I_tmp_wait_iters_ = -(wait_iters);                \
            for (;;) {                                                 \
                wait_expr;                                             \
                                                                       \
                if (recheck) {                                         \
                    goto done;                                         \
                }                                                      \
                                                                       \
                if (++I_tmp_wait_iters_) {                             \
                    break;                                             \
                }                                                      \
            }                                                          \
        }                                                              \
        else {                                                         \
            for (;;) {                                                 \
                wait_expr;                                             \
                                                                       \
                if (recheck) {                                         \
                    goto done;                                         \
                }                                                      \
            }                                                          \
        }                                                              \
    }

#endif

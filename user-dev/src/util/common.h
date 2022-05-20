#ifndef _COMMON_H_
#define _COMMON_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/commonlib.h"
#include "util/attrs.h"
#include "util/macro.h"
#include "util/portability.h"
#include "util/types.h"

#ifdef __cplusplus
#include <type_traits>
#endif


#define ALIGNED(x) __attribute__((aligned(x)))

#define UNUSED(x) ((void)(x))

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

#define IMPOSSIBLE(x)                                                  \
    if (x) {                                                           \
        __builtin_unreachable();                                       \
    }

#define GURANTEED(x) IMPOSSIBLE(!(x))


#define const_assert(...)                                              \
    CAT(I_const_assert_, NOT_ONE_NARG(__VA_ARGS__))(__VA_ARGS__)
#define I_const_assert_ONE(x)       I_static_assert_base(x, "")
#define I_const_assert_MANY(x, ...) I_static_assert_base(x, __VA_ARGS__)

#define choose_const_expr I_choose_const_expr
#define check_has_include I_has_include

#define ensure_const_eval(expr)                                        \
    {                                                                  \
        enum { CAT(CHECKING_CONSTANT_EVAL, __CURRENT__) = (expr) };    \
    }

#endif

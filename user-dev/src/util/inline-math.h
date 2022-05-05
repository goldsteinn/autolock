#ifndef _SRC__UTIL__INLINE_MATH_H_
#define _SRC__UTIL__INLINE_MATH_H_


#include "arch/ll-intrin.h"
#include "util/common.h"


#define ROUNDUP(x, y)   ((y) * (((x) + ((y)-1)) / (y)))
#define ROUNDDOWN(x, y) ((y) * ((x) / (y)))

#define ROUNDUP_P2(x, y)   (((x) + ((y)-1)) & (-(y)))
#define ROUNDDOWN_P2(x, y) ((x) & (-(y)))

#define is_p2(x)     (!((x) & ((x)-1)))
#define p2_factor(x) ((x) & (~((x)-1)))

#define I_cmax2(x, y) ((x) > (y) ? (x) : (y))
#define I_cmin2(x, y) ((x) < (y) ? (x) : (y))

#define I_max2(x, y)                                                           \
    ({                                                                         \
        get_type(x) _tmp_evaluated_x = (x);                                    \
        get_type(y) _tmp_evaluated_y = (y);                                    \
        _tmp_evaluated_x > _tmp_evaluated_y ? _tmp_evaluated_x                 \
                                            : _tmp_evaluated_y;                \
    })

#define I_min2(x, y)                                                           \
    ({                                                                         \
        get_type(x) _tmp_evaluated_x = (x);                                    \
        get_type(y) _tmp_evaluated_y = (y);                                    \
        _tmp_evaluated_x < _tmp_evaluated_y ? _tmp_evaluated_x                 \
                                            : _tmp_evaluated_y;                \
    })


#define MAX(...) APPLY_RECURSE(I_max2, __VA_ARGS__)
#define MIN(...) APPLY_RECURSE(I_min2, __VA_ARGS__)

#define CMAX(...) APPLY_RECURSE(I_cmax2, __VA_ARGS__)
#define CMIN(...) APPLY_RECURSE(I_cmin2, __VA_ARGS__)


#define clz(x)                                                                 \
    choose_const_expr(                                                         \
        sizeof(x) == sizeof(long long), ll_clzll(x),                           \
        choose_const_expr(sizeof(x) == sizeof(long), ll_clzl(x), ll_clz(x)))

#define ctz(x)                                                                 \
    choose_const_expr(                                                         \
        sizeof(x) == sizeof(long long), ll_ctzll(x),                           \
        choose_const_expr(sizeof(x) == sizeof(long), ll_ctzl(x), ll_ctz(x)))


#define popcnt(x)                                                              \
    choose_const_expr(                                                         \
        sizeof(x) == sizeof(long long), __builtin_popcountll(x),               \
        choose_const_expr(sizeof(x) == sizeof(long), __builtin_popcountl(x),   \
                          __builtin_popcount(x)))

#define log2_ru(x) (sizeof_bits(get_type(x)) - clz((x)-1))
#define log2_rd(x) ((sizeof_bits(get_type(x)) - 1) - clz(x))

#define next_p2(x)                                                             \
    ({                                                                         \
        get_type(x) _tmp_evaluated_x = (x);                                    \
        get_type(MAKE_UNSIGNED(_tmp_evaluated_x)) _next_p2_tmp_ =              \
            (_tmp_evaluated_x);                                                \
        /* If constant compiler can calculate this version at compile          \
         * time. */                                                            \
        if (__builtin_constant_p(_next_p2_tmp_)) {                             \
            --_next_p2_tmp_;                                                   \
            _next_p2_tmp_ |= _next_p2_tmp_ >> 1;                               \
            _next_p2_tmp_ |= _next_p2_tmp_ >> 2;                               \
            _next_p2_tmp_ |= _next_p2_tmp_ >> 4;                               \
            _next_p2_tmp_ |= _next_p2_tmp_ >> 8;                               \
            _next_p2_tmp_ |= _next_p2_tmp_ >> 16;                              \
            if (sizeof(_tmp_evaluated_x) == 8) {                               \
                _next_p2_tmp_ |=                                               \
                    _next_p2_tmp_ >> (4 * sizeof(_tmp_evaluated_x));           \
            }                                                                  \
            ++_next_p2_tmp_;                                                   \
        }                                                                      \
        else {                                                                 \
            /* Dont see any clever way to drop the branch. */                  \
            if (UNLIKELY(_next_p2_tmp_ <= 2)) {                                \
                ;                                                              \
            }                                                                  \
            else {                                                             \
                _next_p2_tmp_ =                                                \
                    (CAST(get_type(x), 2) << ((sizeof_bits(get_type(x)) - 1) - \
                                              clz(_next_p2_tmp_ - 1)));        \
            }                                                                  \
        }                                                                      \
        _next_p2_tmp_;                                                         \
    })


#define prev_p2(x)                                                             \
    ({                                                                         \
        get_type(x) _tmp_evaluated_x = (x);                                    \
        get_type(MAKE_UNSIGNED(_tmp_evaluated_x)) _prev_p2_tmp_ =              \
            (_tmp_evaluated_x);                                                \
        /* If constant compiler can calculate this version at compile          \
         * time. */                                                            \
        if (__builtin_constant_p(_prev_p2_tmp_)) {                             \
            _prev_p2_tmp_ |= _prev_p2_tmp_ >> 1;                               \
            _prev_p2_tmp_ |= _prev_p2_tmp_ >> 2;                               \
            _prev_p2_tmp_ |= _prev_p2_tmp_ >> 4;                               \
            _prev_p2_tmp_ |= _prev_p2_tmp_ >> 8;                               \
            _prev_p2_tmp_ |= _prev_p2_tmp_ >> 16;                              \
            if (sizeof(_tmp_evaluated_x) == 8) {                               \
                _prev_p2_tmp_ |=                                               \
                    _prev_p2_tmp_ >> (4 * sizeof(_tmp_evaluated_x));           \
            }                                                                  \
            _prev_p2_tmp_ -= (_prev_p2_tmp_ >> 1);                             \
        }                                                                      \
        else {                                                                 \
            _prev_p2_tmp_ &=                                                   \
                (CAST(get_type(x), 1)                                          \
                 << ((sizeof_bits(get_type(x)) - 1) - clz(_prev_p2_tmp_)));    \
        }                                                                      \
        _prev_p2_tmp_;                                                         \
    })

#endif

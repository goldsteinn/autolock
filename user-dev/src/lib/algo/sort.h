#ifndef _SRC__LIB__ALGO__SORT_H_
#define _SRC__LIB__ALGO__SORT_H_


#include "util/error-util.h"
#include "util/macro.h"
#include "util/types.h"

#ifdef __cplusplus
#include <algorithm>
#define generic_sort(base, nelem) std::sort(base, (base) + (nelem))
#else

#include <math.h>
typedef int32_t (*I_sort_cmp_func)(void const *, void const *);

#define I_type_cmp_name(T) CAT(I_cmp_, T)
#define I_type_cmp(T)                                                          \
    static int32_t I_type_cmp_name(T)(void const * a, void const * b) {        \
        T tmp_a, tmp_b;                                                        \
        __builtin_memcpy(&tmp_a, a, sizeof(tmp_a));                            \
        __builtin_memcpy(&tmp_b, b, sizeof(tmp_b));                            \
        return (tmp_a < tmp_b) ? (-1) : ((tmp_b < tmp_a) ? 1 : 0);             \
    }


APPLY(I_type_cmp, ;, ALL_UNIQUE_TYPES);

#define I_make_type_cmp_ret(T)                                                 \
    T:                                                                         \
    &I_type_cmp_name(T)

#define I_default_cmp_func(x)                                                  \
    Generic(x, APPLY_TOO(I_make_type_cmp_ret, COMMA, ALL_UNIQUE_TYPES),        \
            default                                                            \
            : NULL)


/* Define here so we can add non-null attr and get compile-time error if unknown
 * type is used. */
static NONNULL(1, 4) void I_qsort(void *                base,
                                  uint64_t              nelem,
                                  uint32_t              elem_size,
                                  const I_sort_cmp_func cmp_func) {
    qsort(base, nelem, elem_size, cmp_func);
}

#define I_generic_sort_4(base, nelem, elem_size, cmp_func)                     \
    I_qsort(base, nelem, elem_size, cmp_func)

#define I_generic_sort_3(base, nelem, cmp_func)                                \
    I_qsort(base, nelem, sizeof(base[0]), cmp_func)

#define I_generic_sort_2(base, nelem)                                          \
    I_qsort(base, nelem, sizeof(base[0]), I_default_cmp_func(base[0]))

#define generic_sort(...)                                                      \
    CAT(I_generic_sort_, PP_NARG(__VA_ARGS__))(__VA_ARGS__)
#endif

#endif

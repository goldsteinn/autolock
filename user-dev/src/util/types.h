#ifndef _SRC__UTIL__TYPES_H_
#define _SRC__UTIL__TYPES_H_

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>

#include "util/attrs.h"
#include "util/macro.h"
#include "util/portability.h"

typedef signed char      signed_char;
typedef signed short     signed_short;
typedef signed int       signed_int;
typedef signed long      signed_long;
typedef signed long long signed_long_long;
typedef long double      long_double;

typedef unsigned char      unsigned_char;
typedef unsigned short     unsigned_short;
typedef unsigned int       unsigned_int;
typedef unsigned long      unsigned_long;
typedef unsigned long long unsigned_long_long;

typedef long long long_long;


typedef signed_char *      signed_charp;
typedef signed_short *     signed_shortp;
typedef signed_int *       signed_intp;
typedef signed_long *      signed_longp;
typedef signed_long_long * signed_long_longp;
typedef long_double *      long_doublep;

typedef unsigned_char *      unsigned_charp;
typedef unsigned_short *     unsigned_shortp;
typedef unsigned_int *       unsigned_intp;
typedef unsigned_long *      unsigned_longp;
typedef unsigned_long_long * unsigned_long_longp;

typedef long_long * long_longp;

typedef uint64_t ptr_int_t;


#define SIGNED_INT_TYPES                                               \
    char, signed_char, signed_short, signed_int, signed_long,          \
        signed_long_long
#define SIGNED_FLOAT_TYPES float, double, long_double

#define UNSIGNED_INT_TYPES                                             \
    unsigned_char, unsigned_short, unsigned_int, unsigned_long,        \
        unsigned_long_long
#define UNSIGNED_FLOAT_TYPES

#define ALL_UNIQUE_INT_TYPES   SIGNED_INT_TYPES, UNSIGNED_INT_TYPES
#define ALL_UNIQUE_FLOAT_TYPES SIGNED_FLOAT_TYPES

#define ALL_UNIQUE_TYPES ALL_UNIQUE_INT_TYPES, SIGNED_FLOAT_TYPES

#define ALL_INT_NAMES                                                  \
    wchar_t, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t,    \
        uint64_t, int64_t, short, int, long, long_long,                \
        ALL_UNIQUE_INT_TYPES

#define ALL_TYPE_NAMES ALL_INT_NAMES, ALL_UNIQUE_FLOAT_TYPES


/* Use safe_<type> for aliasing casts. */
#define I_make_safe(T)                                                 \
    typedef T CAT(safe_, T) __attribute__((may_alias))
APPLY(I_make_safe, ;, ALL_TYPE_NAMES);

/* Any types pointer is Tp. */
#define I_make_ptr(T) typedef T * CAT(T, p)
APPLY(I_make_ptr, ;, ALL_TYPE_NAMES);

#define safe_T(T) CAT(safe_, T)
#define ptr_T(T)  CAT(T, p)


#define sizeof_bits(x) (sizeof(x) * CHAR_BIT)

#define CAST_TO_FUNC(func, x) ((FUNC_T(func))(x))
#define FUNC_T(func)          __typeof__(&(func))

#define CAST(x, ...) ((x)(__VA_ARGS__))
#define AGU(base, offset)                                              \
    (CAST(ptr_int_t, base) + CAST(ptr_int_t, offset))
#define AGU_T(base, offset) CAST(get_type(base), AGU(base, offset))
#define PTRDIF(end, begin)                                             \
    (CAST(ptr_int_t, end) - CAST(ptr_int_t, begin))


#define get_type(x)        __typeof__(x)
#define is_same_type(x, y) I_is_same_type(get_type(x), get_type(y))

#define is_same_func_type(decl, func) is_same_type(decl, &(func))


#ifdef __cplusplus


#include <limits>
#include <type_traits>

/* Custom struct instead of just using std::conditional in the macro
 * because the
 * ',' mess up macro parsing. */
template<typename T>
struct I_default_type {
    using type =
        typename std::conditional<std::is_void<T>::value, int, T>::type;
};


template<typename T>
struct I_type_info {
    template<typename Q = T>
    using I_to_signed_T =
        typename std::conditional<std::is_integral<Q>::value,
                                  std::make_signed<Q>,
                                  I_default_type<Q>>::type::type;

    template<typename Q = T>
    using I_to_unsigned_T =
        typename std::conditional<std::is_integral<Q>::value,
                                  std::make_unsigned<Q>,
                                  I_default_type<Q>>::type::type;


    using I_signed_T   = I_to_signed_T<T>;
    using I_unsigned_T = I_to_unsigned_T<T>;

    using I_signed_ptr_T = typename std::conditional<
        std::is_pointer<T>::value,
        typename std::add_pointer<
            I_to_signed_T<typename std::remove_pointer<T>::type>>,
        I_default_type<T>>::type::type;


    using I_unsigned_ptr_T = typename std::conditional<
        std::is_pointer<T>::value,
        typename std::add_pointer<
            I_to_unsigned_T<typename std::remove_pointer<T>::type>>,
        I_default_type<T>>::type::type;


    template<class Q = T>
    static constexpr
        typename std::enable_if<std::is_pointer<Q>::value, Q>::type
        as_ptr(Q x) {
        return x;
    }

    template<class Q = T>
    static constexpr typename std::enable_if<!std::is_pointer<Q>::value,
                                             void **>::type
    as_ptr(Q x) {
        return (void **)(NULL);
    }
};


#define DEFAULT_VALUE(T) CAST(I_AS_NOT_VOID_T(T), 0)

#define IS_PTR(x) std::is_pointer<get_type(x)>::value

#define I_AS_NOT_VOID_T(T) typename I_default_type<T>::type

#define I_PTR_TYPE(x)                                                  \
    typename std::conditional<std::is_pointer<get_type(x)>::value,     \
                              get_type(x), void **>::type


#define sizeof_deref(x)        sizeof(*CAST(I_PTR_TYPE(x), x))
#define I_is_same_type(T0, T1) std::is_same<T0, T1>::value


template<uint64_t N>
struct I_int_of_size {
    using by_byte_t = std::conditional_t<
        N <= 1,
        uint8_t,
        std::conditional_t<
            N <= 2,
            uint16_t,
            std::conditional_t<N <= 4, uint32_t, uint64_t>>>;

    using by_size_t = std::conditional_t<
        N <= UCHAR_MAX,
        uint8_t,
        std::conditional_t<
            N <= USHRT_MAX,
            uint16_t,
            std::conditional_t<N <= UINT_MAX, uint32_t, uint64_t>>>;
};


#define I_AS_PTR(x) (I_type_info<get_type(x)>::as_ptr(x))

#define INT_OF_SIZE_T(x)          I_int_of_size<x>::by_byte_t
#define INT_HAS_CAPACITY_FOR_T(x) I_int_of_size<x>::by_size_t


#define MAKE_UNSIGNED(x)                                               \
    CAST(typename I_type_info<get_type(x)>::I_unsigned_T, x)

#define MAKE_SIGNED(x)                                                 \
    CAST(typename I_type_info<get_type(x)>::I_signed_T, x)

#define MAKE_UNSIGNED_PTR(x)                                           \
    CAST(typename I_type_info<get_type(x)>::I_unsigned_ptr_T, x)

#define MAKE_SIGNED_PTR(x)                                             \
    CAST(typename I_type_info<get_type(x)>::I_signed_ptr_T, x)


#define IS_SIGNED(x) std::is_signed<get_type(x)>::value
#define IS_UNSIGNED_INT(x)                                             \
    (std::is_integral<get_type(x)>::value &&                           \
     std::is_unsigned<get_type(x)>::value)

#define IS_FLOAT_BASE(x) std::is_floating_point<get_type(x)>::value
#define IS_INT_BASE(x)   std::is_integral<get_type(x)>::value

#define TYPEOF_MAX(x) std::numeric_limits<get_type((x))>::max()
#define TYPEOF_MIN(x) std::numeric_limits<get_type((x))>::min()

#else

typedef int32_t bool;
static const bool false = 0;
static const bool true  = !false;


/* Use to make derefencing never a compiler error/warning.  */
#define I_AS_PTR(x)                                                    \
    (I_choose_const_expr(IS_PTR(x), (x), ((void **)(NULL))))


/* GCC does not const evaluate `__builtin_classify_type` if `x` is void.
 */
#define I_AS_NOT_VOID(x)                                               \
    (I_choose_const_expr(I_is_same_type(get_type(x), void), 0, (x)))

/* GCC / LLVM only.  */
#define IS_PTR(x)                                                      \
    (__builtin_classify_type(I_AS_NOT_VOID(x)) ==                      \
     5 /* 5 is ptr type.  */)


#define DEFAULT_VALUE(T)                                               \
    (I_choose_const_expr(I_is_same_type(T, void), 0, CAST(T, 0)))

#define sizeof_deref(x)        sizeof(*(x))
#define I_is_same_type(T0, T1) __builtin_types_compatible_p(T0, T1)


#if STDC_VERSION >= 2011

#define INT_OF_SIZE_T(x)                                               \
    get_type(I_choose_const_expr(                                      \
        x == 1, (uint8_t)0,                                            \
        I_choose_const_expr(                                           \
            x == 2, (uint16_t)0,                                       \
            I_choose_const_expr(x == 4, (uint32_t)0, (uint64_t)0))))

#define INT_HAS_CAPACITY_FOR_T(x)                                      \
    get_type(I_choose_const_expr(                                      \
        CAST(uint64_t, x) <= UCHAR_MAX, (uint8_t)0,                    \
        I_choose_const_expr(                                           \
            CAST(uint64_t, x) <= USHRT_MAX, (uint16_t)0,               \
            I_choose_const_expr(CAST(uint64_t, x) <= UINT_MAX,         \
                                (uint32_t)0, (uint64_t)0))))


// clang-format off
#define MAKE_UNSIGNED(x)                                            \
    Generic((x),                                                    \
            char                       : (unsigned_char)(x),        \
            signed_char                : (unsigned_char)(x),        \
            signed_int                 : (unsigned_int)(x),         \
            signed_short               : (unsigned_short)(x),       \
            signed_long                : (unsigned_long)(x),        \
            signed_long_long           : (unsigned_long_long)(x),   \
            default                    : x)


#define MAKE_UNSIGNED_PTR(x)                                        \
    Generic((x),                                                    \
            char *                     : (unsigned_char *)(x),      \
            signed_char *              : (unsigned_char *)(x),      \
            signed_int *               : (unsigned_int *)(x),       \
            signed_short *             : (unsigned_short *)(x),     \
            signed_long *              : (unsigned_long *)(x),      \
            signed_long_long *         : (unsigned_long_long *)(x), \
            default                    : x)


#define MAKE_SIGNED(x)                                              \
    Generic((x),                                                    \
            unsigned_char              : (char) (x),                \
            unsigned_int               : (signed_int) (x),          \
            unsigned_short             : (signed_short) (x),        \
            unsigned_long              : (signed_long) (x),         \
            unsigned_long_long         : (signed_long_long) (x),    \
            default                    : x)


#define MAKE_SIGNED_PTR(x)                                          \
    Generic((x),                                                    \
            unsigned_char *            : (char *) (x),              \
            unsigned_int *             : (signed_int *) (x),        \
            unsigned_short *           : (signed_short *) (x),      \
            unsigned_long *            : (signed_long *) (x),       \
            unsigned_long_long *       : (signed_long_long *) (x),  \
            default                    : x)

/* TODO: it might be easier to maintain if implement these with
 * APPLY macros based on type classes. */
#define IS_SIGNED(x)                            \
    Generic((x),                                \
            unsigned_char              : 0,     \
            unsigned_short             : 0,     \
            unsigned_int               : 0,     \
            unsigned_long              : 0,     \
            unsigned_long_long         : 0,     \
            signed_char                : 1,     \
            signed_short               : 1,     \
            signed_int                 : 1,     \
            signed_long                : 1,     \
            signed_long_long           : 1,     \
            char                       : 1,     \
            long_double                : 1,     \
            double                     : 1,     \
            float                      : 1,     \
            default                    : 0)


#define TYPEOF_MAX(x)                                   \
    Generic((x),                                        \
            unsigned_char              : UCHAR_MAX,     \
            unsigned_short             : USHRT_MAX,     \
            unsigned_int               : UINT_MAX,      \
            unsigned_long              : ULONG_MAX,     \
            unsigned_long_long         : ULLONG_MAX,    \
            signed_char                : SCHAR_MAX,     \
            signed_short               : SHRT_MAX,      \
            signed_int                 : INT_MAX,       \
            signed_long                : LONG_MAX,      \
            signed_long_long           : LLONG_MAX,     \
            char                       : CHAR_MAX,      \
            long_double                : LDBL_MAX,      \
            double                     : DBL_MAX,       \
            float                      : FLT_MAX,       \
            default                    : 0)

#define TYPEOF_MIN(x)                                   \
    Generic((x),                                        \
            unsigned_char              : 0,             \
            unsigned_short             : 0,             \
            unsigned_int               : 0,             \
            unsigned_long              : 0,             \
            unsigned_long_long         : 0,             \
            signed_char                : SCHAR_MIN,     \
            signed_short               : SHRT_MIN,      \
            signed_int                 : INT_MIN,       \
            signed_long                : LONG_MIN,      \
            signed_long_long           : LLONG_MIN,     \
            char                       : CHAR_MIN,      \
            long_double                : (-LDBL_MAX),   \
            double                     : -(DBL_MAX),    \
            float                      : (-FLT_MAX),    \
            default                    : 0)


#define IS_UNSIGNED_INT(x)                      \
    Generic((x),                                \
            unsigned_char              : 1,     \
            unsigned_short             : 1,     \
            unsigned_int               : 1,     \
            unsigned_long              : 1,     \
            unsigned_long_long         : 1,     \
            signed_char                : 0,     \
            signed_short               : 0,     \
            signed_int                 : 0,     \
            signed_long                : 0,     \
            signed_long_long           : 0,     \
            char                       : 0,     \
            default                    : 0)

#define IS_INT_BASE(x)                          \
    Generic((x),                                \
            unsigned_char              : 1,     \
            unsigned_short             : 1,     \
            unsigned_int               : 1,     \
            unsigned_long              : 1,     \
            unsigned_long_long         : 1,     \
            signed_char                : 1,     \
            signed_short               : 1,     \
            signed_int                 : 1,     \
            signed_long                : 1,     \
            signed_long_long           : 1,     \
            char                       : 1,     \
            default                    : 0)

#define IS_FLOAT_BASE(x)                        \
    Generic((x),                                \
            long_double                : 1,     \
            double                     : 1,     \
            float                      : 1,     \
            default                    : 0)


#define IS_FLOAT_BASE(x)                        \
    Generic((x),                                \
            long_double                : 1,     \
            double                     : 1,     \
            float                      : 1,     \
            default                    : 0)


// clang-format on


#else
/* TODO: Implement the type macro in C99 (not sure how). */
#error "Older standard unsupported"

/* Could start to build around these but so far don't need this suite
 * for any
 * C99 projects and they have issues handling things like `void` input.
 */
#define I_SIGN_PROMOTE(A, B) (1 ? (A) : (B))
#define I_PROMOTE_1(EXPR)    I_SIGN_PROMOTE(1, (EXPR))
#define I_PROMOTE_M1(EXPR)   I_SIGN_PROMOTE(-1, (EXPR))
#define IS_SIGNED(EXPR)      (I_PROMOTE_M1(EXPR) < I_PROMOTE_1(EXPR))

#endif

#endif
#define IS_UNSIGNED(x) (!IS_SIGNED(x))
#define TYPE_MAX(T)    TYPEOF_MAX(DEFAULT_VALUE(T))
#define TYPE_MIN(T)    TYPEOF_MIN(DEFAULT_VALUE(T))

#define GET_IS_FLOAT_BASE(T)   IS_FLOAT_BASE(DEFAULT_VALUE(T))
#define GET_IS_INT_BASE(T)     IS_INT_BASE(DEFAULT_VALUE(T))
#define GET_IS_UNSIGNED(T)     IS_UNSIGNED(DEFAULT_VALUE(T))
#define GET_IS_SIGNED(T)       IS_SIGNED(DEFAULT_VALUE(T))
#define GET_IS_UNSIGNED_INT(T) IS_UNSIGNED_INT(DEFAULT_VALUE(T))
#define GET_IS_PTR(T)          IS_PTR(DEFAULT_VALUE(T))

#define GET_UNSIGNED_T(T) get_type(MAKE_UNSIGNED(DEFAULT_VALUE(T)))
#define GET_UNSIGNED_PTR_T(T)                                          \
    get_type(MAKE_UNSIGNED_PTR(DEFAULT_VALUE(T)))
#define GET_SIGNED_T(T)     get_type(MAKE_SIGNED(DEFAULT_VALUE(T)))
#define GET_SIGNED_PTR_T(T) get_type(MAKE_SIGNED_PTR(DEFAULT_VALUE(T)))


#define FLOAT_TO_INT_T(x) GET_SIGNED_T(INT_OF_SIZE_T(sizeof(x)))

#define I_typedef_func(new_name, existing_name)                        \
    get_type(existing_name) new_name __attribute__((                   \
        alias(V_TO_STR(existing_name)), ATTR_COPY(existing_name)))

#define typedef_func(new_name, existing_name)                          \
    I_typedef_func(new_name, existing_name)

#endif

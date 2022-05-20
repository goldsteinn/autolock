#ifndef _SRC__UTIL__PORTABILITY_H_
#define _SRC__UTIL__PORTABILITY_H_

/* TODO: Do all this with config. */

// clang-format off
#if defined(__STDC__)
# if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 201710L)
#   define STDC_VERSION 2017
#  elif (__STDC_VERSION__ >= 201112L)
#   define STDC_VERSION 2011
#  elif (__STDC_VERSION__ >= 199901L)
#   define STDC_VERSION 1999
#  elif (__STDC_VERSION__ >= 199409L)
#   define STDC_VERSION 1994
#  else
#  define STDC_VERSION 1990
#  endif
# else
# define STDC_VERSION 1989
# endif
#else
# error "NO __STDC__"
#endif
// clang-format on


#if defined(__GNUC__) && !defined(__llvm__) &&                         \
    !defined(__INTEL_COMPILER)
#ifdef __has_include
#define I_has_include __has_include
#endif

/* Keep noclone */
#if __GNUC__ >= 7
#define fall_through __attribute__((fallthrough))
#endif
#if __GNUC__ >= 9
#define I_attribute_copy(...) __copy__(__VA_ARGS__)
#endif

#elif defined __clang__
#ifdef __has_include
#define I_has_include __has_include
#endif
#define noclone
#else
#define noclone
#warning "Untested and likely unsupported compiler"
#endif

#ifndef I_has_include
#define I_has_include(...) 0
#endif

#ifdef __cplusplus
#define extern_C_start() extern "C" {
#define extern_C_end()   }
#if (!defined fall_through) && (__cplusplus >= 201703L)
#define fall_through [[fallthrough]]
#endif

#define restrict
#define I_static_assert_base              static_assert
#define I_choose_const_expr(cond, e0, e1) ((cond) ? (e0) : (e1))
#else
#define extern_C_start()
#define extern_C_end()

#define constexpr
#define I_static_assert_base _Static_assert

#define I_choose_const_expr(cond, e0, e1)                              \
    __builtin_choose_expr(cond, e0, e1)

#if STDC_VERSION >= 2011
#define Generic(...) _Generic(__VA_ARGS__)
#else
#error "Older standard unsupported"
#endif

#endif


#ifndef fall_through
#define fall_through /* fall through */
#endif

#ifndef I_attribute_copy
#define I_attribute_copy(...)
#endif

#endif

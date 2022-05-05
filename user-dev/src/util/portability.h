#ifndef _SRC__UTIL__PORTABILITY_H_
#define _SRC__UTIL__PORTABILITY_H_

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


#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
/* Keep noclone */
#if __GNUC__ >= 7
#define fall_through __attribute__((fallthrough))
#endif

#elif defined __clang__
#define noclone
#else
#define noclone
#warning "Untested and likely unsupported compiler"
#endif

#ifdef __cplusplus
#if (!defined fall_through) && (__cplusplus >= 201703L)
#define fall_through [[fallthrough]]
#endif

#define restrict
#define I_static_assert_base              static_assert
#define I_choose_const_expr(cond, e0, e1) ((cond) ? (e0) : (e1))
#else
#define constexpr
#define I_static_assert_base              _Static_assert
#define I_choose_const_expr(cond, e0, e1) __builtin_choose_expr(cond, e0, e1)

#if STDC_VERSION >= 2011
#define Generic(...) _Generic(__VA_ARGS__)
#else
#error "Older standard unsupported"
#endif

#endif


#ifndef fall_through
#define fall_through /* fall through */
#endif

#endif

#ifndef _SRC__UTIL__ERROR_UTIL_H_
#define _SRC__UTIL__ERROR_UTIL_H_


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "util/common.h"
#include "util/macro.h"
#include "util/verbosity.h"

#define err_assert(...)                                                \
    CAT(err_assert_, NOT_ONE_NARG(__VA_ARGS__))(__VA_ARGS__)
#define die_assert(...)                                                \
    CAT(die_assert_, NOT_ONE_NARG(__VA_ARGS__))(__VA_ARGS__)


#define warn(msg, ...) fprint(stderr, "warning: " msg, ##__VA_ARGS__)
#define warn_once(msg, ...)                                            \
    ({                                                                 \
        static bool I_tmp_warned_;                                     \
        if (I_tmp_warned_ == 0) {                                      \
            I_tmp_warned_ = 1;                                         \
            warn(msg, ##__VA_ARGS__);                                  \
        }                                                              \
    })

#define warn_once_assert(X, msg, ...)                                  \
    if (UNLIKELY(!(X))) {                                              \
        warn_once(msg, ##__VA_ARGS__);                                 \
    }

#define msg_assert(X, msg, args...)                                    \
    if (UNLIKELY(!(X))) {                                              \
        _msg_die(msg, ##args);                                         \
    }

#define warn_assert(X, ...)                                            \
    if (UNLIKELY(!(X))) {                                              \
        warn(__VA_ARGS__);                                             \
    }

#define err_assert_MANY(X, msg, args...)                               \
    if (UNLIKELY(!(X))) {                                              \
        errdie(msg, ##args);                                           \
    }

#define err_assert_ONE(X)                                              \
    if (UNLIKELY(!(X))) {                                              \
        errdie(NULL);                                                  \
    }

#define die_assert_MANY(X, msg, args...)                               \
    if (UNLIKELY(!(X))) {                                              \
        die(msg, ##args);                                              \
    }

#define die_assert_ONE(X)                                              \
    if (UNLIKELY(!(X))) {                                              \
        die(NULL);                                                     \
    }

#define die(msg, args...)    I_die(ERR_ARGS, msg, ##args);
#define errdie(msg, args...) I_errdie(ERR_ARGS, errno, msg, ##args);

//#define WITH_DBG_PRINT
#ifdef WITH_DBG_PRINT
#define dbg_assert(...) die_assert(__VA_ARGS__)
#define dbg_print(...)  fprintf(stderr, __VA_ARGS__)
#define PRINTFFL        fprintf(stderr, "%-20s:%-20s:%-4d\n", ERR_ARGS)
#else
#define dbg_print(...)
#define dbg_assert(...)
#define PRINTFFL
#endif

EXIT_FUNC NONNULL(1,
                  2) void I_va_errdie(char const * restrict file_name,
                                      char const * restrict func_name,
                                      uint32_t line_number,
                                      int32_t  error_number,
                                      char const * restrict msg,
                                      va_list ap);

EXIT_FUNC NONNULL(1, 2)
    FORMATF(5, 6) void I_errdie(char const * restrict file_name,
                                char const * restrict func_name,
                                uint32_t line_number,
                                int32_t  error_number,
                                char const * restrict msg,
                                ...);

EXIT_FUNC NONNULL(1, 2)
    FORMATF(4, 5) void I_die(char const * restrict file_name,
                             char const * restrict func_name,
                             uint32_t line_number,
                             char const * restrict msg,
                             ...);

EXIT_FUNC FORMATF(1, 2) void I_msg_die(char const * restrict msg, ...);
#endif

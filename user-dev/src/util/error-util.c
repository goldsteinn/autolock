#include <string.h>

#include "util/error-util.h"

enum { I_STRERROR_BUFLEN = 512 };

#define I_strerror(err)                                                       \
    ({                                                                         \
        char I_tmp_err_buf_[I_STRERROR_BUFLEN];                              \
        if (strerror_r(err, I_tmp_err_buf_, I_STRERROR_BUFLEN)) {            \
            memcpy_c(I_tmp_err_buf_, "Error generating strerror msg!",        \
                     strlen("Error generating strerror msg!"));                \
        }                                                                      \
        I_tmp_err_buf_;                                                       \
    })

void
I_va_errdie(char const * restrict file_name,
            char const * restrict func_name,
            uint32_t line_number,
            int32_t  error_number,
            char const * restrict msg,
            va_list ap) {
    fprintf(stderr, "%s:%s:%d: [%d] -> %s\n", file_name, func_name, line_number,
            error_number, I_strerror(error_number));
    if (msg) {
        /* va_list warning is a clang-tidy bug */
        vfprintf(stderr, /* NOLINT */
                 msg,    /* NOLINT */
                 ap);    /* NOLINT */
    }
    abort();
}

void
I_errdie(char const * restrict file_name,
         char const * restrict func_name,
         uint32_t line_number,
         int32_t  error_number,
         char const * restrict msg,
         ...) {
    fprintf(stderr, "%s:%s:%d: [%d] -> %s\n", file_name, func_name, line_number,
            error_number, I_strerror(error_number));
    va_list ap;
    if (msg) {
        va_start(ap, msg);
        /* va_list warning is a clang-tidy bug */
        vfprintf(stderr, /* NOLINT */
                 msg,    /* NOLINT */
                 ap);    /* NOLINT */
        va_end(ap);
    }
    abort();
}

void
I_die(char const * restrict file_name,
      char const * restrict func_name,
      uint32_t line_number,
      char const * restrict msg,
      ...) {
    fprintf(stderr, "%s:%s:%d\n", file_name, func_name, line_number);
    va_list ap;
    if (msg) {
        va_start(ap, msg);
        /* va_list warning is a clang-tidy bug */
        vfprintf(stderr, /* NOLINT */
                 msg,    /* NOLINT */
                 ap);    /* NOLINT */
        va_end(ap);
    }
    abort();
}

void
I_msg_die(char const * restrict msg, ...) {
    va_list ap;
    if (msg) {
        va_start(ap, msg);
        /* va_list warning is a clang-tidy bug */
        vfprintf(stderr, /* NOLINT */
                 msg,    /* NOLINT */
                 ap);    /* NOLINT */
        va_end(ap);
    }
    abort();
}

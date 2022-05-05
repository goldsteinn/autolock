#include "util/regex-util.h"

#include "util/error-util.h"


NONNULL(2, 3)
static uint64_t
I_re_error(int32_t errcode,
           regex_t const * restrict re,
           char * restrict errbuf,
           uint64_t errbuf_size) {
    return regerror(errcode, re, errbuf, errbuf_size);
}


EXIT_FUNC void
I_re_errdie(int32_t errcode,
            regex_t const * restrict re,
            char const * restrict file_name,
            char const * restrict func_name,
            uint32_t line_num) {
    enum { ERRBUF_LEN = 512 };
    char errbuf[ERRBUF_LEN];
    I_re_error(errcode, re, errbuf, ERRBUF_LEN);
    I_die(file_name, func_name, line_num, "Regex Error (%u) -> %s\n", errcode,
          errbuf);
}


int32_t
I_safe_re_compile(regex_t * restrict re,
                  char const * restrict pattern,
                  int32_t cflags,
                  char const * restrict file_name,
                  char const * restrict func_name,
                  uint32_t line_num) {
    int32_t ret = re_compile(re, pattern, cflags);
    if (UNLIKELY(ret)) {
        I_re_errdie(ret, re, file_name, func_name, line_num);
    }
    return ret;
}

int32_t
I_safe_re_exec(regex_t const * restrict re,
               char const * restrict pattern,
               uint64_t   nmatch,
               regmatch_t pmatch[restrict],
               int        eflags,
               char const * restrict file_name,
               char const * restrict func_name,
               uint32_t line_num) {
    int32_t ret = re_exec(re, pattern, nmatch, pmatch, eflags);
    if (UNLIKELY(ret != 0 && ret != REG_NOMATCH)) {
        I_re_errdie(ret, re, file_name, func_name, line_num);
    }
    return ret;
}

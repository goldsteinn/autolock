#include <stdarg.h>

#include "util/verbosity.h"

static int32_t I_verbose_ = 0;

int32_t
check_verbosity(int32_t verbosity_bound) {
    return verbosity_bound <= I_verbose_;
}

int32_t
get_verbosity() {
    return I_verbose_;
}

void
set_verbosity(int32_t verbosity) {
    I_verbose_ = verbosity;
}

void
vcheck_print(int32_t verbosity_bound, FILE * fp, const char * msg, ...) {
    if (check_verbosity(verbosity_bound)) {
        va_list ap;
        va_start(ap, msg);
        /*  No lint for clang-tidy bug.  */
        vfprintf(fp,  /* NOLINT */
                 msg, /* NOLINT */
                 ap); /* NOLINT */
        va_end(ap);
    }
}

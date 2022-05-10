#ifndef _SRC__THREAD__RSEQ__RSEQ_H_
#define _SRC__THREAD__RSEQ__RSEQ_H_

#include "thread/rseq/rseq-defs.h"
#include "thread/rseq/rseq-portable.h"

#include "arch/ll-syscall.h"

#include "util/common.h"
#include "util/error-util.h"
#include "util/types.h"

extern __thread struct rseq I_rseq_area;
extern __thread uint32_t    I_rseq_is_init;


static uint32_t
rseq_is_init() {
    return I_rseq_is_init;
}

static void
I_rseq_set_is_init() {
    I_rseq_is_init = 1;
}

static int32_t
rseq_init_first() {
    int32_t ret;

    rseq_glibc_prepare();
    ret = ll_syscall_cc(
        I_NR_rseq, (&I_rseq_area, sizeof(I_rseq_area), 0, I_RSEQ_SIG),
        /* no +m */, /* no m */,
        /* no =m */);
    if (LIKELY(ret == 0)) {
        I_rseq_set_is_init();
    }
    else {
        rseq_is_supported(ret);
    }
    return ret;
}

static int32_t
rseq_init() {
    if (LIKELY(rseq_is_init())) {
        return 0;
    }

    return rseq_init_first();
}


#endif

#ifndef _SRC__THREAD__RSEQ__RSEQ_GLIBC_SUPPORT_H_
#define _SRC__THREAD__RSEQ__RSEQ_GLIBC_SUPPORT_H_


#if check_has_include("linux/rseq.h")
#define I_HAS_RSEQ
#elif check_has_include("linux/version.h")
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
#define I_HAS_RSEQ
#else
#error "restartable sequences not supported before linux 4.18."
#endif
#else
/* Need to check at runtime. */
#endif

#ifdef I_HAS_RSEQ
static void
rseq_is_supported(int32_t syscall_ret) {
    (void)(syscall_ret);
}
#else
#include "util/error-util.h"

static uint32_t
rseq_is_supported(int32_t syscall_ret) {
    die_assert(syscall_ret != -ENOSYS, "rseq not supported\n");
}
#endif


#endif

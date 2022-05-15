#include "autolock-impls/autolock-kernel-abi.h"
#include "util/common.h"
extern_C_start();
/* Don't change this name unless you also change rseq TLS access
 * symbols. */
__thread struct kernel_autolock_abi * I_kernel_autolock;
extern_C_end();

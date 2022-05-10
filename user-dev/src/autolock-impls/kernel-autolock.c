#include "autolock-impls/autolock-kernel-abi.h"

/* Don't change this name unless you also change rseq TLS access
 * symbols. */
__thread struct kernel_autolock_abi * I_kernel_autolock;

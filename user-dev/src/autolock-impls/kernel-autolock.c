#include "autolock-impls/autolock-kernel-abi.h"
#include "util/common.h"
extern_C_start();

#ifdef I_USE_FOR_SHARED_LIBRARY_INTERPOSE
#error "This file should never be linked with shared library"
#endif


/* Don't change this name unless you also change rseq TLS access
 * symbols. */
__thread struct kernel_autolock_abi * I_kernel_autolock
    ATTR_TLS_INIT_EXEC;
extern_C_end();

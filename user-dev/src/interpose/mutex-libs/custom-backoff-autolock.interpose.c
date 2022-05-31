#include "autolock-impls/locks/custom-backoff-autolock.h"
#include "interpose/pthread-mutex-interpose.h"
#include "util/attrs.h"

extern_C_start();

#ifndef I_USE_FOR_SHARED_LIBRARY_INTERPOSE
#error                                                                 \
    "Interpose library has incorrect defines and will have incorrect TLS storage settings."
#endif

__thread struct kernel_autolock_abi * I_kernel_autolock
    ATTR_TLS_INIT_EXEC;


create_pthread_interpose(custom_backoff_autolock);
extern_C_end();

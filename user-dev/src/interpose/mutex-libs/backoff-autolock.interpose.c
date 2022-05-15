
#include "autolock-impls/backoff-autolock.h"
#include "interpose/pthread-mutex-interpose.h"

extern_C_start();
__thread struct kernel_autolock_abi * I_kernel_autolock;


create_pthread_interpose(backoff_autolock);
extern_C_end();

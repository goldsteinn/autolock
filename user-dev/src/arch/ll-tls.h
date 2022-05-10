#ifndef _SRC__ARCH__LL_TLS_H_
#define _SRC__ARCH__LL_TLS_H_

#include <features.h>

#if __GNUC_PREREQ(11, 1)
static void *
ll_tls_start() {
    return __builtin_thread_pointer();
}
#elif defined(__x86_64__)
#include "arch/x86/x86-tls.h"
#else
#error "Unsupported arch"
#endif


#endif

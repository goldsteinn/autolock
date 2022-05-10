#ifndef _SRC__ARCH__X86__X86_TLS_H_
#define _SRC__ARCH__X86__X86_TLS_H_


#include <stdint.h>

static void *
ll_tls_start() {
    void * p;
    __asm__("mov %%fs:0, %[p]" : [p] "=r"(p) : :);
    return p;
}


#endif

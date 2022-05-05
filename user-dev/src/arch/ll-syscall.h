#ifndef _SRC__ARCH__LL_SYSCALL_H_
#define _SRC__ARCH__LL_SYSCALL_H_

#include <sys/syscall.h>

#ifdef __x86_64__
#include "arch/x86/x86-syscall.h"
#else
#include <unistd.h>
#define ll_syscall            syscall
#define ll_syscall_cc(x, ...) ll_syscall(x)
#endif


#endif

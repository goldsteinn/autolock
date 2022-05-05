#ifndef _SRC__ARCH__LL_GETCPU_H_
#define _SRC__ARCH__LL_GETCPU_H_


#if (defined __x86_64__)
#include "arch/x86/x86-getcpu.h"
#else
#include <unistd.h>
#define _get_cpu sched_getcpu
#endif

#endif

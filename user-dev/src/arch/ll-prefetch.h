#ifndef _SRC__ARCH__LL_PREFETCH_H_
#define _SRC__ARCH__LL_PREFETCH_H_

#ifdef __x86_64__
#include "arch/x86/x86-prefetch.h"
#else

/* Doesn't break anything to skip these. */
#define ll_prefetchw(...)
#define ll_prefetch_L1(...)
#define ll_prefetch_L2(...)
#define ll_prefetch_L3(...)

#endif
#endif

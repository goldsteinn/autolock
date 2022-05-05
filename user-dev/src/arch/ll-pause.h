#ifndef _SRC__ARCH__LL_PAUSE_H_
#define _SRC__ARCH__LL_PAUSE_H_

#ifdef __x86_64__
#include "x86/x86-pause.h"

#else
#error "Unsupported arch"
#endif

#endif

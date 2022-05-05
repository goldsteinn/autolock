#ifndef _SRC__ARCH__INL_ASM_H_
#define _SRC__ARCH__INL_ASM_H_

#ifdef __x86_64__
#include "arch/x86/x86-inl-asm.h"
#else
#error "Unsupported architecture"
#endif

#endif

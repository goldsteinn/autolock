#ifndef _SRC__ASM__LIBC_ASM_COMMON_H_
#define _SRC__ASM__LIBC_ASM_COMMON_H_

#ifdef USE_AS_SSE2
#include "libc-defs/libc-sse2-vecs.h"
#elif defined USE_AS_AVX
#include "libc-defs/libc-avx-vecs.h"
#elif defined USE_AS_AVX2
#include "libc-defs/libc-avx2-vecs.h"
#elif defined USE_AS_EVEX
#include "libc-defs/libc-evex-vecs.h"
#elif defined USE_AS_AVX512
#include "libc-defs/libc-avx512-vecs.h"
#endif

#define USE_MULTIARCH
#define IS_IN(x) 1

#define ZERO_UPPER_VEC_REGISTERS_RETURN VZEROUPPER_RETURN
#define VZEROUPPER_RETURN                                                      \
    VZEROUPPER;                                                                \
    ret

#define RAX_LP rax
#define RDI_LP rdi
#define RSI_LP rsi
#define RDX_LP rdx
#define RCX_LP rcx

#define R8_LP r8
#define R9_LP r9

#define R10_LP r10
#define R11_LP r11
#define R12_LP r12
#define R13_LP r13

#define _CET_ENDBR   endbr4
#define _CET_NOTRACK notrack

#include "asm-common.h"

#endif

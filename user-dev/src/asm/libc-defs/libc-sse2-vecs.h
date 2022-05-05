#ifndef _SRC__ASM__LIBC_SSE2_VECS_H_
#define _SRC__ASM__LIBC_SSE2_VECS_H_

#ifndef HAS_VECS
#define HAS_VECS      1
#define USE_WITH_SSE2 1
#define SECTION(p)    p##.sse2
#endif

#define MOV_SIZE 3
#define VMOVNT   movntdq
#define VMOVU    movups
#define VMOVA    movaps

#define VZEROUPPER_SIZE 0
#define VZEROUPPER

#define VEC_SIZE 16
#define VEC(i)   xmm##i

#endif

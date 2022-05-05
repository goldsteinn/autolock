#ifndef _SRC__ARCH__X86__X86_GETCPU_H_
#define _SRC__ARCH__X86__X86_GETCPU_H_

#ifdef __RDPID__
#define ll_get_cpu __builtin_ia32_rdpid

#else
#include <unistd.h>
#define ll_get_cpu sched_getcpu

#endif


#endif

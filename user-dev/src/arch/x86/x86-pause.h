#ifndef _SRC__ARCH__X86__X86_PAUSE_H_
#define _SRC__ARCH__X86__X86_PAUSE_H_


#define ll_pause() __asm__ volatile("pause" : : :)


#endif

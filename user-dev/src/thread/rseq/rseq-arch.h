#ifndef _SRC__THREAD__RSEQ__RSEQ_SIGNATURE_H_
#define _SRC__THREAD__RSEQ__RSEQ_SIGNATURE_H_


#ifdef __x86_64__
#include "thread/rseq/x86/x86-rseq-arch.h"
#else
#error "Unsupported architecture"
#endif


#endif

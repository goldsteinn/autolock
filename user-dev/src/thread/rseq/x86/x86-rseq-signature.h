#ifndef _SRC__THREAD__RSEQ__X86__X86_RSEQ_SIGNATURE_H_
#define _SRC__THREAD__RSEQ__X86__X86_RSEQ_SIGNATURE_H_

/* RSEQ_SIG is a signature required before each abort handler code.

   RSEQ_SIG is used with the following reserved undefined instructions,
   which trap in user-space:

   x86-32:    0f b9 3d 53 30 05 53      ud1    0x53053053,%edi
   x86-64:    0f b9 3d 53 30 05 53      ud1    0x53053053(%rip),%edi  */

#define I_RSEQ_SIG 0x53053053


#endif

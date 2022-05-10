#ifndef _SRC__THREAD__RSEQ__X86__X86_RSEQ_ASM_DEFS_H_
#define _SRC__THREAD__RSEQ__X86__X86_RSEQ_ASM_DEFS_H_


#include "util/macro.h"

/*
 * label 1 -> begin criical section (include cpu comparison)
 * label 2 -> end critical section
 * label 3 -> rseq info strcut
 * label 4 -> abort sequence
 */

/* defines the info struct used for control flow */
#define RSEQ_AREA_DEF(alignment)                                       \
    ".pushsection __rseq_cs, \"aw\"\n\t" /* creation section. */       \
    ".balign " V_TO_STR(alignment) ""    /* alignment at least 32. */  \
    "\n\t"                                                             \
    "3:\n\t"           /* struct rseq_area start.*/                    \
                                                                       \
    ".long 0x0\n"     /* version = 0. */                               \
    ".long 0x0\n"     /* flags = 0. */                                 \
    ".quad 1f\n"      /* start_ip = 1f (label 1, forward) */           \
    ".quad 2f - 1f\n" /* post_commit_offset = (start_cs::label -       \
                         end_cs::label) */                             \
    ".quad 4f\n"      /* abort label = 4f (label 4) */                 \
    ".popsection\n\t" /* end section */


#define RSEQ_CUR_CPU() "%%fs:I_rseq_area@tpoff+4"

#define RSEQ_CS_ARR_DEF()                                              \
    ".pushsection __rseq_cs_ptr_array, \"aw\"\n\t" /* create ptr       \
                                                      section. */      \
    ".quad 3b\n\t"    /* set ptr to addr of rseq_area. */              \
    ".popsection\n\t" /* end section. */


#define RSEQ_PREP_CS_DEF(TMP_REGISTER)                                 \
    "leaq 3b (%%rip), " V_TO_STR(TMP_REGISTER) "\n\t" /* get start of I_rseq_area. */ \
    "movq " V_TO_STR(TMP_REGISTER) ", %%fs:I_rseq_area@tpoff+8\n\t" /* store in ptr field in I_rseq_area. */


#define RSEQ_START_ABORT_DEF()                                         \
    ".pushsection __rseq_failure, \"ax\"\n\t" /* create failure        \
                                                 section. */           \
    ".byte 0x0f, 0xb9, 0x3d\n\t"                                       \
    ".long 0x53053053\n\t"                                             \
    "4:\n\t" /* abort label. */

#define RSEQ_END_ABORT_DEF()                                           \
    ".popsection\n\t" /* end failure section. */


#endif

#ifndef _SRC__THREAD__RSEQ__RSEQ_DEFS_H_
#define _SRC__THREAD__RSEQ__RSEQ_DEFS_H_

#include "util/common.h"

/* Since we may include <sys/rseq.h> which include <linux/rseq.h> must
 * use <linux/rseq.h> */
#if check_has_include("linux/rseq.h")
#include <linux/rseq.h>
#else
#include "thread/rseq/rseq-kernel-abi.h"
#endif

/* Arch specific signature and asm macros */
#include "thread/rseq/rseq-arch.h"

#define RSEQ_INIT_NEW_CS(reg)                                          \
    RSEQ_AREA_DEF() RSEQ_CS_ARR_DEF() RSEQ_PREP_CS_DEF(reg)

#define I_rseq_alignment 32
enum { I_NR_rseq = 334 };

const_assert(sizeof(struct rseq) == I_rseq_alignment);

#endif

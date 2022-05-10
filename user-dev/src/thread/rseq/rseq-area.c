#include "thread/rseq/rseq-defs.h"

__thread struct rseq I_rseq_area ALIGNED(32);
__thread uint32_t                I_rseq_is_init = 0;

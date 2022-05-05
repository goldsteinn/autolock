#ifndef _SRC__BENCH__BENCH_COMMON_H_
#define _SRC__BENCH__BENCH_COMMON_H_

#include "util/attrs.h"
#include "util/common.h"
#include "util/time-util.h"
#include "util/types.h"

#define compiler_barrier() __asm__ volatile("" : : : "memory")
#define compiler_do_not_optimize_out(x)                                        \
    __asm__ volatile("" : : "r,m,v"(x) : "memory")
#define compiler_code_align(x) __asm__ volatile(".align " V_TO_STR(x) "" : : :);

#endif

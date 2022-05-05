#include <stdint.h>

#include "util/func-decl-generator.h"
#include "util/macro.h"

#include "bench/bench-decls.h"

#define decl_benchmark(name) extern void * name(void *);

make_decls(benchmarks, decl_benchmark, BENCHMARKS);

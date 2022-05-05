#include "bench/bench-common.h"
#include "bench/bench-generator.h"

#include "util/inline-math.h"
#include "util/macro.h"

ll_make_tput_bench(bench_next_p2,
                   trials,
                   next_p2(i),
                   uint32_t trials,
                   uint32_t i);
ll_make_tput_bench(bench_prev_p2,
                   trials,
                   prev_p2(i),
                   uint32_t trials,
                   uint32_t i);


#define run_bench(res_name, res_time, func)                                    \
    (res_name) = V_TO_STR(func);                                               \
    (res_time) = CAT(bench_, func)(trials, i);


void *
bench_p2(void * bench_args) {
    uint32_t trials = CAST(uint32_t, CAST(uint64_t, bench_args));
    uint32_t i      = 0;

    uint64_t     times[4] = { 0 };
    const char * names[4] = { 0 };

    enum { DEFAULT_TRIALS = 10 * 1000 * 1000 };
    if (!trials) {
        trials = DEFAULT_TRIALS;
    }

    run_bench(names[0], times[0], next_p2);
    run_bench(names[1], times[1], prev_p2);

    print_res(names[0], times[0], trials, get_ll_units());
    print_res(names[1], times[1], trials, get_ll_units());

    return 0;
}

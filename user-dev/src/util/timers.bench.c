#include "bench/bench-common.h"

#include "util/time-util.h"

#define run_bench(res_name, res_time, func, ...)                               \
    start = get_ll_time();                                                     \
    for (i = trials; i; --i) {                                                 \
        compiler_do_not_optimize_out(func(__VA_ARGS__));                       \
    }                                                                          \
    end      = get_ll_time();                                                  \
    res_name = V_TO_STR(func);                                                 \
    res_time = get_ll_dif(end, start);

static ALWAYS_INLINE uint64_t
vdso_direct_get_ns(const FUNC_T(clock_gettime) vdso_direct) {
    struct timespec ts;
    (void)vdso_direct(CLOCK_THREAD_CPUTIME_ID, &ts);
    return to_ns(&ts);
}

void *
bench_timers(void * bench_args) {
    uint32_t  trials = CAST(uint32_t, CAST(uint64_t, bench_args));
    ll_time_t start, end;
    uint32_t  i;

    enum { NUM_TIMERS = 6, DEFAULT_TRIALS = 1000 * 1000 };

    uint64_t     times[NUM_TIMERS]    = { 0 };
    const char * names[NUM_TIMERS]    = { 0 };
    FUNC_T(clock_gettime) vdso_direct = get_vdso_clock_gettime();

    if (!trials) {
        trials = DEFAULT_TRIALS;
    }

    run_bench(names[0], times[0], vdso_get_ns);
    safe_vdso_init();
    names[0] = "vdso_get_ns_preinit";
    run_bench(names[1], times[1], vdso_get_ns);
    run_bench(names[2], times[2], get_ns);
    run_bench(names[3], times[3], direct_get_ns);
    run_bench(names[4], times[4], get_ll_time);
    run_bench(names[5], times[5], vdso_direct_get_ns, vdso_direct);

    for (i = 0; i < NUM_TIMERS; ++i) {
        print_res(names[i], times[i], trials, get_ll_units());
    }

    return 0;
}

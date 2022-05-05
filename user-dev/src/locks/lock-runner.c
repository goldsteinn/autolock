#include "locks/lock-runner.h"

#include "test/test-common.h"
#include "thread/thread-common.h"

static global_counter_t I_global_state;


static uint64_t *
add_time(uint64_t * times, void * time) {
    if (times == NULL) {
        return NULL;
    }
    *times = CAST(uint64_t, time);
    return times + 1;
}

static global_counter_t
get_expected_count(uint32_t num_threads,
                   uint32_t outer_iter,
                   uint32_t cs_iter) {
    return CAST(global_counter_t, num_threads) *
           CAST(global_counter_t, outer_iter) * CAST(global_counter_t, cs_iter);
}

static int32_t
I_run(func_decl_t const * to_run,
      run_params_t *      params,
      stats_result_t *    result_out) {
    uint32_t         i, j;
    global_counter_t expec;
    uint32_t         num_trials  = params->num_trials;
    uint32_t         num_threads = params->num_threads;
    uint64_t *       times, *times_start;
    void *           res;

    thread_t      tids[num_threads];
    thread_attr_t attr;


    vprint("%s Lock (%s). Num Trials = %u, Num Threads = %u\n",
           result_out ? "Benchmarking" : "Testing", to_run->name, num_trials,
           num_threads);

    params->global_state = &I_global_state;

    times = NULL;
    if (result_out) {
        times =
            (uint64_t *)safe_calloc(num_threads * num_trials, sizeof(uint64_t));
    }
    times_start = times;


    expec =
        get_expected_count(num_threads, params->outer_iter, params->cs_iter);
    safe_thread_attr_init(&attr);
    safe_thread_attr_set_stacksize(&attr, 32768);

    for (i = 0; i < num_trials; ++i) {
        run_params_reset(params);

        for (j = 0; j < num_threads; ++j) {
            safe_thread_create(tids + j, &attr, to_run->bench_func, params);
        }
        for (j = 0; j < num_threads; ++j) {
            safe_thread_join(tids[j], &res);
            times = add_time(times, res);
        }


        test_assert(expec == *(params->global_state),
                    "%-16s(%u): Bad count %lu vs %lu\n", to_run->name,
                    num_threads, expec, *(params->global_state));
    }

    if (result_out) {
        gen_stats(result_out, times_start, times - times_start);
        safe_sfree(times_start, PTRDIF(times, times_start));
    }

    return 0;
}


int32_t
run(func_decl_t const * to_run,
    run_params_t *      params,
    stats_result_t *    result_out) {
    die_assert(to_run);
    /* The price we pay for c99. */
    die_assert(params);
    return I_run(to_run, params, result_out);
}

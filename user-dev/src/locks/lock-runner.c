#include "locks/lock-runner.h"

#include "test/test-common.h"
#include "thread/thread-common.h"

static global_counter_t I_global_state;

static void
cpu_list_to_cpuset(char const * cpu_list, cpuset_t * cset) {
    char *   next_c;
    uint32_t cpu;
    uint32_t nprocs = get_num_cpus();
    cset_zero(cset);

    die_assert(cpu_list != NULL, "empty cpu_list\n");

    for (;;) {
        cpu = strtol(cpu_list, &next_c, 10);
        if (cpu_list == next_c) {
            break;
        }

        if (cpu < nprocs) {
            cset_set(cpu, cset);
        }
        else {
            warn_once("Specified cpu [%u] out of range [0 - %u]\n", cpu,
                 nprocs - 1);
        }


        if (*next_c == '\0') {
            break;
        }

        cpu_list = next_c + 1;
    }

    die_assert(!cset_empty(cset));
}

static int32_t
should_pin_cpus(char const * cpu_list,
                int32_t      num_cpus,
                uint32_t     pin_cpus) {
    return cpu_list != NULL || (num_cpus > 0) || pin_cpus;
}

static int32_t
set_allowable_cset(cpuset_t *   cset,
                   char const * cpu_list,
                   int32_t      num_cpus,
                   uint32_t     pin_cpus,
                   uint32_t     prefer_hyper_threads) {
    if (!should_pin_cpus(cpu_list, num_cpus, pin_cpus)) {
        cset_zero(cset);
        return 0;
    }
    else if (cpu_list != NULL) {
        warn_once_assert(
            num_cpus <= 0,
            "Overriding --num-cpus preference with explicit cpu list\n");
        warn_once_assert(
            prefer_hyper_threads == 0,
            "Overriding --prefer-hyper-threads selection with explicit cpu list\n");

        cpu_list_to_cpuset(cpu_list, cset);
    }
    else if (num_cpus <= 0) {
        cset_all(cset);
    }
    else {
        int32_t nprocs = get_num_cpus();
        if (num_cpus >= nprocs) {
            warn_once("--num-cpus (%d) greater than number of procs (%d)\n",
                 num_cpus, nprocs);
            cset_all(cset);
        }
        else {
            cset_zero(cset);
            if (prefer_hyper_threads) {
                /* Set sibling groups at a time. This is relatively
                 * inefficient, especially considering the current
                 * implementation to find siblings, but its off the
                 * critical path. We could precompute much of the cpu
                 * info stuff at compile time and make this efficient if
                 * need be. */
                cpuset_t cset_siblings;
                int32_t  i, num_set = num_cpus;
                cset_zero(&cset_siblings);

                for (i = 0; i < nprocs; ++i) {
                    if (cset_isset(i, cset)) {
                        continue;
                    }

                    get_logical_cpu_ids(i, &cset_siblings);
                    num_set = cset_count(&cset_siblings);
                    if (num_set >= num_cpus) {
                        break;
                    }
                    num_cpus -= num_set;
                    /* Should be impossible for us to have any of
                     * the siblings. */
                    die_assert(cset_test(&cset_siblings, cset) == 0);

                    cset_or(cset, &cset_siblings);
                }
                die_assert(cset_test(&cset_siblings, cset) == 0);

                /* Amount remaining. */
                num_set -= num_cpus;
                for (i = 0; num_set && i < nprocs; ++i) {
                    if (cset_isset(i, &cset_siblings)) {
                        cset_clr(i, &cset_siblings);
                        --num_set;
                    }
                }
                cset_or(cset, &cset_siblings);
            }
            else {
                /* Avoiding grouping by hyper thread. */
                int32_t i, phys_id, iter_cnt = 0;
                for (;;) {
                    for (i = 0; i < nprocs; ++i) {
                        if (cset_isset(i, cset)) {
                            continue;
                        }

                        phys_id = get_phys_cpu_id(i);
                        if (i == phys_id || iter_cnt != 0) {
                            cset_set(i, cset);
                            if ((--num_cpus) == 0) {
                                goto done;
                            }
                        }
                    }
                    iter_cnt = 1;
                }
            done:
                (void)(0);
            }
        }
    }
    return 1;
}

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
           CAST(global_counter_t, outer_iter) *
           CAST(global_counter_t, cs_iter);
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
    int32_t       with_cpu_pinning;
    cpuset_t      cur_cset, allowable_cset;


    vprint("%s Lock (%s). Num Trials = %u, Num Threads = %u\n",
           result_out ? "Benchmarking" : "Testing", to_run->name,
           num_trials, num_threads);

    params->global_state = &I_global_state;

    times = NULL;
    if (result_out) {
        times = (uint64_t *)safe_calloc(num_threads * num_trials,
                                        sizeof(uint64_t));
    }
    times_start = times;


    expec = get_expected_count(num_threads, params->outer_iter,
                               params->cs_iter);


    with_cpu_pinning = set_allowable_cset(
        &allowable_cset, params->cpu_list, params->num_cpus,
        params->pin_cpus, params->prefer_hyper_threads);
    die_assert(cset_empty(&allowable_cset) == (!with_cpu_pinning));
    cset_zero(&cur_cset);

    safe_thread_attr_init(&attr);
    safe_thread_attr_set_stacksize(&attr, 32768);


    for (i = 0; i < num_trials; ++i) {
        run_params_reset(params);


        for (j = 0; j < num_threads; ++j) {
            if (with_cpu_pinning) {
                cpuset_t cset;
                if (cset_empty(&cur_cset)) {
                    cset_copy(&cur_cset, &allowable_cset);
                }
                cset_copy_first(&cset, &cur_cset);
                die_assert(!cset_empty(&cset));
                cset_clr_first(&cur_cset);

                safe_thread_attr_set_affinity(&attr, &cset);
            }
            safe_thread_create(tids + j, &attr, to_run->bench_func,
                               params);
        }
        for (j = 0; j < num_threads; ++j) {
            safe_thread_join(tids[j], &res);
            times = add_time(times, res);
        }


        test_assert(expec == *(params->global_state),
                    "%-16s(%u): Bad count %lu vs %lu\n", to_run->name,
                    num_threads, expec, *(params->global_state));
    }
    safe_thread_attr_destroy(&attr);

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

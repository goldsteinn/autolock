#ifndef _SRC__LOCKS__LOCKS_LOCK_RUN_PARAMS_H_
#define _SRC__LOCKS__LOCKS_LOCK_RUN_PARAMS_H_

#include "util/error-util.h"
#include "util/memory-util.h"

#include "thread/barrier.h"

typedef uint64_t global_counter_t;
typedef struct run_params {
    /* Used in benchmark. */
    void *             shared_memory;
    global_counter_t * global_state;
    uint32_t           outer_iter;
    uint32_t           cs_iter;
    uint32_t           extra_iter;
    thread_barrier_t   barrier;

    /* Bookkeeping. */
    uint32_t num_trials;
    uint32_t num_threads;

    char const * cpu_list;
    int32_t     num_cpus;
    uint32_t     pin_cpus;
    uint32_t     prefer_hyper_threads;

} run_params_t;


static void
run_params_reset(run_params_t * params) {
    die_assert(params);
    die_assert(params->shared_memory);
    die_assert(params->global_state);

    memset_c(params->shared_memory, 0, PAGE_SIZE);
    memset_c(params->global_state, 0, sizeof(*(params->global_state)));
}

static void
run_params_init(run_params_t * params,
                uint32_t       outer_iter,
                uint32_t       cs_iter,
                uint32_t       extra_iter,
                /* Bookkeeping. */
                uint32_t     num_trials,
                uint32_t     num_threads,
                char const * cpu_list,
                int32_t     num_cpus,
                uint32_t     pin_cpus,
                uint32_t     prefer_hyper_threads) {
    void * p;
    die_assert(params);

    __builtin_memset(params, 0, sizeof(*params));

    /* Just grab a page. */
    p = safe_aligned_alloc(PAGE_SIZE, PAGE_SIZE);

    params->shared_memory = p;
    params->outer_iter    = outer_iter;
    params->cs_iter       = cs_iter;
    params->extra_iter    = extra_iter;

    safe_thread_barrier_init(&(params->barrier), NULL, num_threads);

    params->num_trials           = num_trials;
    params->num_threads          = num_threads;
    params->cpu_list             = cpu_list;
    params->num_cpus             = num_cpus;
    params->pin_cpus             = pin_cpus;
    params->prefer_hyper_threads = prefer_hyper_threads;
}


static void
run_params_destroy(run_params_t * params) {
    die_assert(params);

    safe_sfree(params->shared_memory, PAGE_SIZE);
    safe_thread_barrier_destroy(&(params->barrier));
}


#endif

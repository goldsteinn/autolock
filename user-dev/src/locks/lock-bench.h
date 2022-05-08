#ifndef _SRC__LOCK_BENCH_H_
#define _SRC__LOCK_BENCH_H_

#include "arch/ll-timing.h"

#include "bench/bench-common.h"

#include "locks/lock-run-params.h"

#include "thread/barrier.h"


#define I_cs_work(state)    (++(*(state)))
#define I_extra_work(state) (++(state))

template<typename lock_T>
uint64_t
I_bench_runner_kernel(lock_T *           lock,
                      global_counter_t * global_state,
                      uint32_t           outer_iter,
                      uint32_t           cs_iter,
                      uint32_t           extra_iter,
                      thread_barrier_t * barrier) {
    uint32_t  i, local_state = 0;
    ll_time_t start, end;
    thread_barrier_wait(barrier);
    start = get_ll_time();
    compiler_barrier();
    assert(outer_iter);
    GURANTEED(outer_iter);

    for (; outer_iter; --outer_iter) {
        lock->lock();
        assert(cs_iter);
        for (i = cs_iter; i; --i) {
            compiler_do_not_optimize_out(I_cs_work(global_state));
        }
        lock->unlock();
        for (i = extra_iter; i; --i) {
            compiler_do_not_optimize_out(I_extra_work(local_state));
        }
    }
    compiler_barrier();
    end = get_ll_time();
    return get_ll_dif(end, start);
}

template<typename lock_T>
void *
bench_runner(void * arg) {
    void *  res;
    int32_t is_unique;

    run_params_t *     params        = (run_params_t *)arg;
    void *             shared_memory = params->shared_memory;
    global_counter_t * global_state  = params->global_state;
    uint32_t           outer_iter    = params->outer_iter;
    uint32_t           cs_iter       = params->cs_iter;
    uint32_t           extra_iter    = params->extra_iter;
    thread_barrier_t * barrier       = &(params->barrier);

    lock_T * lock;
    PRINTFFL;
    is_unique = thread_barrier_wait(barrier);
    PRINTFFL;
    lock = (is_unique == THREAD_BARRIER_IS_UNIQUE)
               ? lock_T::init(shared_memory)
               : CAST(lock_T *, shared_memory);
    PRINTFFL;
    res = CAST(void *, I_bench_runner_kernel<lock_T>(
                           lock, global_state, outer_iter, cs_iter,
                           extra_iter, barrier));
    PRINTFFL;
    is_unique = thread_barrier_wait(barrier);
    PRINTFFL;
    if (is_unique == THREAD_BARRIER_IS_UNIQUE) {
        PRINTFFL;
        lock->destroy();
    }
    PRINTFFL;
    return res;
}


#endif

#ifndef _SRC__LOCK_BENCH_H_
#define _SRC__LOCK_BENCH_H_

#include "arch/ll-timing.h"

#include "bench/bench-common.h"

#include "locks/lock-run-params.h"

#include "thread/barrier.h"

#include "thread/rseq/rseq.h"


#define I_cs_work(state)    (++(*(state)))
#define I_extra_work(state) (++(state))

/* Benchmark code run by each thread. Templated per lock so should have
 * minimal overhead. */
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

    /* Initialize rseq. Used for some locks. This is done during TLS on
     * newer GLIBC distros but we may need to do manually. */
    //    die_assert(rseq_init() == 0);

    /* Wait for all threads to be ready before starting. Note this is
     * implemented with a futex. We may get more precision with a second
     * spinlock barrier so more threads are likely to be context
     * switched in. */
    thread_barrier_wait(barrier);

    /* Start time. */
    start = get_ll_time();
    /* Don't reorder start timing with the loop. */
    compiler_barrier();

    /* To avoid branch before entering loop. */
    GURANTEED(outer_iter);

    /* The idea is we have two sets of worth for each iterations. cs
     * (critical section) work and extra work (not in lock). The
     * rationale for extra work is we want the lock to no only be fast
     * but also not interfere with other work done by other threads. */
    for (; outer_iter; --outer_iter) {
        /* Lock. */
        lock->lock();

        /* Run critical section work (in lock). */
        for (i = cs_iter; i; --i) {
            compiler_do_not_optimize_out(I_cs_work(global_state));
        }

        /* Unlock. */
        lock->unlock();

        /* Run extra work. */
        for (i = extra_iter; i; --i) {
            compiler_do_not_optimize_out(I_extra_work(local_state));
        }
    }
    /* Dont reorder end timing with the loop. */
    compiler_barrier();

    /* End time. */
    end = get_ll_time();

    /* Return diff (get get_ll_units() for units. On x86_64 its cycles).
     */
    return get_ll_dif(end, start);
}


/* Benchmark entry. Will initialize the lock_T and run the kernel
 * function. */
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


    /* Thread barrier will return THREAD_BARRIER_IS_UNIQUE for exactly
     * one thread. Use this thread for initialization / destruction of
     * the lock_T object. */
    is_unique = thread_barrier_wait(barrier);
    lock      = (is_unique == THREAD_BARRIER_IS_UNIQUE)
                    ? lock_T::init(shared_memory)
                    : CAST(lock_T *, shared_memory);

    /* Setup done run the benchmark. */
    res = CAST(void *, I_bench_runner_kernel<lock_T>(
                           lock, global_state, outer_iter, cs_iter,
                           extra_iter, barrier));

    is_unique = thread_barrier_wait(barrier);
    if (is_unique == THREAD_BARRIER_IS_UNIQUE) {
        lock->destroy();
    }
    PRINTFFL;
    return res;
}


#endif

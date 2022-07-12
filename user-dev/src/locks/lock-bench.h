#ifndef _SRC__LOCK_BENCH_H_
#define _SRC__LOCK_BENCH_H_

#include "arch/ll-timing.h"

#include "bench/bench-common.h"

#include "locks/lock-run-params.h"

#include "thread/barrier.h"

#include "thread/rseq/rseq.h"

#include "autolock-impls/stats/autolock-stats.h"


#define I_cs_work(state)    (++(*(state)))
#define I_extra_work(state) (++(state))
#define I_cur_global_state(state)                                      \
    (__atomic_load_n(state, __ATOMIC_RELAXED))
#define I_work_done(titer, cstate) ((titer) == (cstate))

static uint32_t
I_fast_rand(uint32_t v) {
    uint64_t c;
    c = v ^ 0x53c5ca59u;
    c *= 0x74743c5bul;
    return c ^ (c >> 32);
}

#define I_FAST_RAND_INIT_VAL(tid)                                      \
    ((uint32_t)(0xa0761d6478bd642f + (tid)))

/* TODO: Rewrite in C and remove absl/folly dependency.  */


/* Benchmark for testing how long each thread takes to do a certain
 * amount of work.  I.e each thread is given "outer_iters * cs_iters"
 * work todo.  The work requires the lock.  We get a seperate time for
 * each thread.  Emphasizes fairness as a thread getting starved will
 * result in higher max.  */
template<typename lock_T>
static uint64_t
I_bench_per_thread_work(lock_T *           lock,
                        global_counter_t * global_state,
                        uint32_t           outer_iter,
                        uint32_t           cs_iter,
                        uint32_t           extra_iter,
                        thread_barrier_t * barrier) {
    uint32_t  i, local_state = 0;
    ll_time_t start, end;
#if 0
    /* Initialize rseq. Used for some locks. This is done during TLS on
     * newer GLIBC distros but we may need to do manually. */
    //    NO RSEQ for now
    //    die_assert(rseq_init() == 0);
#endif

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

/* Throughput benchmark.  There is a set total amount of
 * "total_iter" WORK done increments of "cs_iter".  All thread may make
 * progress whenever they hold the lock.  When all the work is done the
 * benchmark is done. De-emphasizes fairness as a thread getting starved
 * may help the benchmark.  */
template<typename lock_T, uint32_t varied_contention>
static uint64_t
I_bench_total_work(lock_T *           lock,
                   global_counter_t * global_state,
                   uint32_t           total_iter,
                   uint32_t           cs_iter,
                   uint32_t           extra_iter,
                   uint32_t           contention_mask,
                   thread_barrier_t * barrier) {
    uint32_t  i, local_state = 0, rand_state;
    ll_time_t start, end;

    (void)(rand_state);
    (void)(contention_mask);

    if constexpr (varied_contention) {
        rand_state = I_FAST_RAND_INIT_VAL(pthread_self());
    }

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

    /* The idea is we have two sets of worth for each iterations. cs
     * (critical section) work and extra work (not in lock). The
     * rationale for extra work is we want the lock to no only be fast
     * but also not interfere with other work done by other threads. */

    for (;;) {
        global_counter_t cur_state;
        uint32_t         idx = 0;

        if constexpr (varied_contention) {
            rand_state = I_fast_rand(rand_state);
            idx        = rand_state & contention_mask;
        }


        /* Lock. */
        (lock + idx)->lock();

        cur_state = I_cur_global_state(global_state);
        if (UNLIKELY(I_work_done(total_iter, cur_state))) {
            (lock + idx)->unlock();
            break;
        }

        /* Run critical section work (in lock). */
        for (i = cs_iter; i; --i) {
            compiler_do_not_optimize_out(I_cs_work(global_state));
        }

        /* Unlock. */
        (lock + idx)->unlock();

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
    void *            res;
    int32_t           is_unique;
    auto_lock_stats_t kstats;
    run_params_t *    params = (run_params_t *)arg;
    uint8_t * shared_memory  = CAST(uint8_t *, params->shared_memory);
    global_counter_t * global_state     = params->global_state;
    uint32_t           with_sched_stats = params->with_sched_stats;
    uint32_t           outer_iter       = params->outer_iter;
    uint32_t           cs_iter          = params->cs_iter;
    uint32_t           extra_iter       = params->extra_iter;
    uint32_t           contention_mask  = params->contention_mask;
    uint32_t           bench_type       = params->bench_type;
    thread_barrier_t * barrier          = &(params->barrier);

    lock_T * lock;
    uint32_t i;
    uint32_t num_locks = BENCH_TOTAL_WORK ? contention_mask : 0;

    /* Thread barrier will return THREAD_BARRIER_IS_UNIQUE for exactly
     * one thread. Use this thread for initialization / destruction of
     * the lock_T object. */
    is_unique = thread_barrier_wait(barrier);
    if (is_unique == THREAD_BARRIER_IS_UNIQUE) {
        for (i = 0; i <= num_locks; ++i) {
            lock_T::init(shared_memory + i * sizeof(lock_T));
        }
    }

    lock = CAST(lock_T *, shared_memory);

    if (with_sched_stats && is_unique) {
        autolock_start_stats();
    }

    /* Setup done run the benchmark. */
    if (bench_type == BENCH_TOTAL_WORK) {

        if (contention_mask == 0) {
            res = CAST(void *, I_bench_total_work<lock_T, 0>(
                                   lock, global_state, outer_iter,
                                   cs_iter, extra_iter, 0, barrier));
        }
        else {
            res = CAST(void *,
                       I_bench_total_work<lock_T, 0>(
                           lock, global_state, outer_iter, cs_iter,
                           extra_iter, contention_mask, barrier));
        }
    }
    else {
        res = CAST(void *, I_bench_per_thread_work<lock_T>(
                               lock, global_state, outer_iter, cs_iter,
                               extra_iter, barrier));
    }

    PRINTFFL;
    is_unique = thread_barrier_wait(barrier);
    PRINTFFL;
    if (is_unique == THREAD_BARRIER_IS_UNIQUE) {
        for (i = 0; i <= num_locks; ++i) {
            (lock + i)->destroy();
        }
    }
    PRINTFFL;
    if (with_sched_stats && is_unique) {
        autolock_get_stats(&kstats);
        autolock_stats_out(stdout, &kstats);
    }

    PRINTFFL;
    return res;
}


#endif

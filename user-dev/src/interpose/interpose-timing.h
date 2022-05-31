#ifndef _SRC__INTERPOSE__INTERPOSE_TIMING_H_
#define _SRC__INTERPOSE__INTERPOSE_TIMING_H_

#include "arch/ll-timing.h"
#include "util/common.h"

#ifdef I_WITH_TIMING

static char const * I_get_lock_name();
#define I_make_I_get_lock_name(lock_name)                              \
    static char const * I_get_lock_name() {                            \
        return V_TO_STR(lock_name);                                    \
    }


/* Skip first I_WARMUP_CALLS before start time. Use last of
 * I_COOLDOWN_CALLS for end time. */
enum { I_WARMUP_CALLS = 64, I_COOLDOWN_CALLS = 64 };

static __thread uint32_t I_warmup_count = I_WARMUP_CALLS;


typedef struct I_thread_stats {
    struct I_thread_stats * next;

    uint64_t time_start;
    uint64_t time_history[I_COOLDOWN_CALLS];

    uint64_t time_trylock;
    uint64_t time_lock;
    uint64_t time_unlock;

    uint32_t num_calls_trylock;
    uint32_t num_calls_lock;
    uint32_t num_calls_unlock;

    uint32_t history_idx;

} I_thread_stats_t;

static uint32_t                    I_has_printed_hdr = 0;
static I_thread_stats_t *          I_thread_stats_l  = NULL;
static __thread I_thread_stats_t * I_thread_stats    = NULL;
#define I_THREAD_STATS_END CAST(I_thread_stats_t *, -1UL)


static I_thread_stats_t *
I_thread_stats_out(I_thread_stats_t * stats) {
    I_thread_stats_t * next = stats->next;
    uint64_t           time_total, time_end, time_start;
    uint32_t           last_hist_idx;
    if (stats->num_calls_trylock == 0 && stats->num_calls_lock == 0 &&
        stats->num_calls_unlock == 0) {
        goto done;
    }

    last_hist_idx = (stats->history_idx - 1) % I_COOLDOWN_CALLS;

    time_end   = stats->time_history[last_hist_idx];
    time_start = stats->time_start;


    if (UNLIKELY(time_end <= time_start)) {
        fprintf(stderr,
                "No times for: \"%s\" -- Error end before begin\n",
                I_get_lock_name());
        goto done;
    }
    time_total = time_end - time_start;

    if (__atomic_exchange_n(&I_has_printed_hdr, 1, __ATOMIC_RELAXED) ==
        0) {
        fprintf(
            stderr,
            "Lock_Function,Trylock_Calls,Lock_Calls,Unlock_Calls,"
            "Time_Total,Trylock_Time,Lock_Time,Unlock_Time,Units\n");
    }
    fprintf(stderr, "%s,%u,%u,%u,%lu,%lu,%lu,%lu,%s\n",
            I_get_lock_name(), stats->num_calls_trylock,
            stats->num_calls_lock, stats->num_calls_unlock, time_total,
            stats->time_trylock, stats->time_lock, stats->time_unlock,
            get_ll_units());
done:
    return next;
}


static I_thread_stats_t *
I_thread_stats_add() {
    /* NB: We have a memory leak. Probably simple enough to fix but some
     * due-diligence about whether a thread can be active after shared
     * library destructor called is needed first. */
    I_thread_stats_t * stats =
        (I_thread_stats_t *)calloc(1, sizeof(I_thread_stats_t));
    assert(stats != NULL);
    for (;;) {
        I_thread_stats_t * cas_expec;
        cas_expec =
            __atomic_load_n(&I_thread_stats_l, __ATOMIC_RELAXED);
        /* We are at exit. No perf penality printing to stdout so
         * just get the numbers out there. */
        if (cas_expec == I_THREAD_STATS_END) {
            I_thread_stats_out(stats);
            free(stats);
            stats = NULL;
            break;
        }

        stats->next = cas_expec;
        if (__atomic_compare_exchange_n(&I_thread_stats_l, &cas_expec,
                                        stats, false /* strong. */,
                                        __ATOMIC_RELAXED,
                                        __ATOMIC_RELAXED)) {
            break;
        }
    }
    return stats;
}

#define I_get_timer(func) CAT(time_, func)
#define I_get_count(func) CAT(num_calls_, func)


#define timing_start_lock(func)

#define timing_start(func) uint64_t I_func_timing_start = get_ll_time();

#define timing_end(func, ...)                                          \
    uint64_t I_func_timing_end = get_ll_time();                        \
    if (UNLIKELY(I_warmup_count != 0)) {                               \
        if (UNLIKELY(I_thread_stats == NULL)) {                        \
            I_thread_stats = I_thread_stats_add();                     \
        }                                                              \
        --I_warmup_count;                                              \
        I_thread_stats->time_start = I_func_timing_start;              \
    }                                                                  \
    else {                                                             \
        I_thread_stats->I_get_count(func) += 1;                        \
        I_thread_stats->I_get_timer(func) +=                           \
            (I_func_timing_end - I_func_timing_start);                 \
        __VA_ARGS__;                                                   \
    };

#define timing_end_unlock(func)                                        \
    timing_end(                                                        \
        func,                                                          \
        I_thread_stats->time_history[(I_thread_stats->history_idx++) % \
                                     I_COOLDOWN_CALLS] =               \
            I_func_timing_end;)


static __attribute__((destructor)) void
I_destroy_timing() {
    I_thread_stats_t * stats;
    stats = __atomic_exchange_n(&I_thread_stats_l, I_THREAD_STATS_END,
                                __ATOMIC_RELAXED);

    for (; stats != NULL; stats = I_thread_stats_out(stats)) {
    }
}


#else

#define I_make_I_get_lock_name(...)
#define timing_start(...)
#define timing_end(...)
#define timing_end_unlock(...)

#endif


#endif

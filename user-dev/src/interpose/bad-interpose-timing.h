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
static __thread uint32_t I_history_idx;

static __thread uint64_t I_time_start                     = 0;
static __thread uint64_t I_time_history[I_COOLDOWN_CALLS] = { 0 };

static __thread uint64_t I_time_trylock = 0;
static __thread uint64_t I_time_lock    = 0;
static __thread uint64_t I_time_unlock  = 0;

static __thread uint32_t I_num_calls_trylock = 0;
static __thread uint32_t I_num_calls_lock    = 0;
static __thread uint32_t I_num_calls_unlock  = 0;


static uint32_t I_has_printed_hdr = 0;

typedef struct I_thread_stats {
    struct I_thread_stats * next;

    uint64_t time_total;

    uint64_t time_trylock;
    uint64_t time_lock;
    uint64_t time_unlock;

    uint32_t num_calls_trylock;
    uint32_t num_calls_lock;
    uint32_t num_calls_unlock;

    uint32_t total_count;
    uint32_t pid;
    uint32_t ppid;
    uint64_t tid;

} I_thread_stats_t;


static I_thread_stats_t * I_thread_stats_l = NULL;
#define I_THREAD_STATS_END CAST(I_thread_stats_t *, -1UL)

/* We need to interpose the pthread_exit/pthread_join symbol to
 * aggregate the stats.  */
static FUNC_T(pthread_join) I_pthread_join_fp;
static FUNC_T(pthread_exit) I_pthread_exit_fp;


static I_thread_stats_t *
I_thread_stats_out(I_thread_stats_t * stats) {
    I_thread_stats_t * next = stats->next;
    if (__atomic_exchange_n(&I_has_printed_hdr, 1, __ATOMIC_RELAXED) ==
        0) {
        fprintf(
            stderr,
            "Lock_Function,Trylock_Calls,Lock_Calls,Unlock_Calls,"
            "Time_Total,Trylock_Time,Lock_Time,Unlock_Time,Units\n");
    }
    fprintf(stderr, "%s,%u,%u,%lu,%u,%u,%u,%u,%lu,%lu,%lu,%lu,%s\n",
            I_get_lock_name(), stats->pid, stats->ppid, stats->tid,
            stats->total_count, stats->num_calls_trylock,
            stats->num_calls_lock, stats->num_calls_unlock,
            stats->time_total, stats->time_trylock, stats->time_lock,
            stats->time_unlock, get_ll_units());

    free(stats);
    return next;
}


static I_thread_stats_t *
I_thread_stats_new() {
    if (UNLIKELY(I_warmup_count)) {
        return NULL;
    }
    else {
        uint32_t last_hist_idx = (I_history_idx - 1) % I_COOLDOWN_CALLS;

        uint64_t time_end   = I_time_history[last_hist_idx];
        uint64_t time_start = I_time_start;

        if (UNLIKELY(time_end <= time_start)) {
            fprintf(stderr,
                    "No times for: \"%s\" -- Error end before begin\n",
                    I_get_lock_name());
            return NULL;
        }
        uint64_t           time_total = time_end - time_start;
        I_thread_stats_t * stats =
            (I_thread_stats_t *)calloc(1, sizeof(I_thread_stats_t));
        assert(stats != NULL);


        stats->time_total = time_total;

        stats->time_trylock = I_time_trylock;
        stats->time_lock    = I_time_lock;
        stats->time_unlock  = I_time_unlock;

        stats->num_calls_trylock = I_num_calls_trylock;
        stats->num_calls_lock    = I_num_calls_lock;
        stats->num_calls_unlock  = I_num_calls_unlock;

        stats->total_count = I_total_count;
        stats->pid         = getpid();
        stats->ppid        = getppid();
        stats->tid         = pthread_self();
        return stats;
    }
}

static void
I_thread_stats_add() {
    I_thread_stats_t * stats = I_thread_stats_new();
    if (stats != NULL) {
        for (;;) {
            I_thread_stats_t * cas_expec;
            cas_expec =
                __atomic_load_n(&I_thread_stats_l, __ATOMIC_RELAXED);
            /* We are at exit. No perf penality printing to stdout so
             * just get the numbers out there. */
            if (cas_expec == I_THREAD_STATS_END) {
                I_thread_stats_out(stats);
                break;
            }

            stats->next = cas_expec;
            if (__atomic_compare_exchange_n(
                    &I_thread_stats_l, &cas_expec, stats,
                    false /* strong. */, __ATOMIC_RELAXED,
                    __ATOMIC_RELAXED)) {
                break;
            }
        }
    }
}

__attribute__((noreturn)) void
pthread_exit(void * retval) {
    I_thread_stats_add();
    I_pthread_exit_fp(retval);
    assert(0 && "Should be unreachable!");
}

int
pthread_join(pthread_t thread, void ** retval) {
    I_thread_stats_add();
    return I_pthread_join_fp(thread, retval);
}

#define I_get_timer(func) CAT(I_time_, func)
#define I_get_count(func) CAT(I_num_calls_, func)


#define timing_start_lock(func)

#define timing_start(func) uint64_t I_func_timing_start = get_ll_time();

#define timing_end(func, ...)                                          \
    uint64_t I_func_timing_end = get_ll_time();                        \
    if (UNLIKELY(I_warmup_count != 0)) {                               \
        --I_warmup_count;                                              \
        I_time_start = I_func_timing_start;                            \
    }                                                                  \
    else {                                                             \
        I_get_count(func) += 1;                                        \
        I_get_timer(func) +=                                           \
            (I_func_timing_end - I_func_timing_start);                 \
        __VA_ARGS__;                                                   \
    };

#define timing_end_unlock(func)                                        \
    timing_end(func,                                                   \
               I_time_history[(I_history_idx++) % I_COOLDOWN_CALLS] =  \
                   I_func_timing_end;)


static __attribute__((constructor)) void
I_init_timing() {
    I_pthread_join_fp =
        CAST_TO_FUNC(pthread_join, dlsym(RTLD_NEXT, "pthread_join"));
    I_pthread_exit_fp =
        CAST_TO_FUNC(pthread_exit, dlsym(RTLD_NEXT, "pthread_exit"));
    assert(I_pthread_join_fp != NULL);
    assert(I_pthread_exit_fp != NULL);
}

static __attribute__((destructor)) void
I_destroy_timing() {
    I_thread_stats_t *stats, *last_stats;
    I_thread_stats_add();
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

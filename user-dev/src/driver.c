#include "util/arg.h"
#include "util/common.h"
#include "util/error-util.h"
#include "util/func-decl-utils.h"
#include "util/verbosity.h"

#include "locks/lock-export.h"
#include "locks/lock-runner.h"

static int32_t verbosity = 0;

static int32_t do_list  = 0;
static int32_t do_test  = 0;
static int32_t do_bench = 0;

static int32_t stats_csv        = 0;
static int32_t with_sched_stats = 0;

static uint32_t contention = 0;
static uint32_t outer_iter = 1000 * 1000;
static uint32_t cs_iter    = 1;
static uint32_t extra_iter = 0;

static uint32_t     bench_per_thread_work = 0;
static uint32_t     bench_total_work      = 0;
static uint32_t     num_trials            = 1;
static int32_t      num_threads           = 1;
static char const * cpu_list              = NULL;
static int32_t      num_cpus              = -1;
static int32_t      pin_cpus              = 0;
static int32_t      prefer_hyper_threads  = 0;

static int32_t    run_all    = 0;
static arg_rest_t lock_names = INIT_ARG_REST_T;


static ArgOption args[] = {
    /* ADD_ARG(Kind, Method, name, reqd, variable, help) */
    ADD_ARG(KindOption,
            Integer,
            "-v",
            0,
            &verbosity,
            "Turn on verbosity"),
    ADD_ARG(KindOption, Set, "--list", 0, &do_list, "List all locks"),
    ADD_ARG(KindOption, Set, "--bench", 0, &do_bench, "Run benchmarks"),
    ADD_ARG(KindOption, Set, "--test", 0, &do_test, "Run tests"),
    ADD_ARG(KindOption,
            Set,
            "--csv",
            0,
            &stats_csv,
            "Display stats as CSV (only relevent if benchmarking)"),
    ADD_ARG(KindOption,
            Set,
            "--all",
            0,
            &run_all,
            "Run all lock versions"),
    ADD_ARG(KindOption,
            Set,
            ("--sched-stats"),
            0,
            &with_sched_stats,
            "With kernel scheduling info."),
    ADD_ARG(KindOption,
            Set,
            ("--bench-per-thread-work"),
            0,
            &bench_per_thread_work,
            "Run per thread work benchmark."),
    ADD_ARG(KindOption,
            Set,
            ("--bench-total-work"),
            0,
            &bench_total_work,
            "Run total work benchmark."),
    ADD_ARG(KindOption,
            Integer,
            ("--bench-contention", "--contention"),
            0,
            &contention,
            "Contention in total work mode. Must be power of 2."),
    ADD_ARG(
        KindOption,
        String,
        ("--cpus", "--cpu-set"),
        0,
        &cpu_list,
        "Set of cpus to run on. Either csv or bitmap. If contains comma will interpret as csv"),
    ADD_ARG(KindOption,
            Integer,
            ("--ncpus", "--num-cpus"),
            0,
            &num_cpus,
            "Max number of cpus to use. (-1) to use all."),
    ADD_ARG(
        KindOption,
        Set,
        ("--prefer-hyper", "--prefer-hyper-threads"),
        0,
        &prefer_hyper_threads,
        "Set to make core pinning prefer hyperthreads (only relevant if max cpus specified is less than number supported by processors"),
    ADD_ARG(
        KindOption,
        Set,
        ("--pin", "--pin-cpus"),
        0,
        &pin_cpus,
        "Set pin cpus. Will round-robin the threads to available cpus. Available cpus is default all online processors."),
    ADD_ARG(KindOption,
            Integer,
            ("--iter", "--outer-iter"),
            0,
            &outer_iter,
            "Num iterators of driver loop (include cs + extra)"),
    ADD_ARG(KindOption,
            Integer,
            ("--cs", "--cs-iter"),
            0,
            &cs_iter,
            "Num iterations in critical section"),
    ADD_ARG(KindOption,
            Integer,
            ("--extra", "--extra-iter"),
            0,
            &extra_iter,
            "Num iterations out of critical section"),
    ADD_ARG(
        KindOption,
        Integer,
        ("-T", "--threads"),
        0,
        &num_threads,
        "Num of threads to run with. -1 (default) will test up to number of CPUs."),
    ADD_ARG(KindOption,
            Integer,
            ("-t", "--trials"),
            0,
            &num_trials,
            "Number of trials. -1 will dynamically choose a default."),
    ADD_ARG(KindHelp, Help, ("-h", "--help"), 0, NULL, ""),
    ADD_ARG(
        KindRest,
        Rest,
        "",
        0,
        &lock_names,
        "Benchmarks to run. Just include at end (superseded by --all)"),
    ADD_ARG(KindEnd, EndOptions, "", 0, NULL, "")
};


static ArgDefs argp = { args, "Bench Driver", NULL, NULL };


static run_params_t      common_params;
static stats_result_t ** stats;
static uint32_t          stats_idx;


static void
run_lock_test(func_decl_t const * lock) {
    die_assert(run(lock, &common_params, stats + stats_idx) == 0);
    if (stats) {
        stats_set_desc(stats[stats_idx++], lock->name);
    }
}
int
main(int argc, char * argv[]) {
    die_assert(!doParse(&argp, argc, argv),
               "Error parsing arguments\n");
    set_verbosity(verbosity);

    /* Check arguments are valid. */
    die_assert(do_bench + do_test + do_list == 1,
               "Must specify list, test, or bench (exclusively)\n");
    die_assert(num_threads > 0,
               "Must have positive number of threads\n");
    die_assert(num_trials > 0, "Must have positive number of trials\n");
    die_assert(outer_iter > 0, "Must have non-zero outer-iter count\n");

    die_assert(bench_total_work);
    if (bench_per_thread_work == 0 && bench_total_work == 0) {
        bench_per_thread_work = 1;
    }
    die_assert(
        bench_per_thread_work + bench_total_work == 1,
        "Can only run per thread work or total work benchmarks. Not both.\n");

    die_assert(!(bench_per_thread_work && contention),
               "Contention set with per thread benchmarking!\n");

    die_assert(!(contention & (contention - 1)),
               "Contention is not a power of 2!\n");

    if (do_list || (!run_all && lock_names.n == 0)) {
        list_decls_filtered(
            &lock_list, run_all ? NULL : lock_names.ptrs, lock_names.n);
        return 0;
    }
    else if (do_bench) {
        stats = (stats_result_t **)safe_calloc(
            lock_list.ndecls, sizeof(stats_result_t *));
    }
    run_params_init(
        &common_params, with_sched_stats, outer_iter, cs_iter,
        extra_iter, contention,
        bench_total_work ? BENCH_TOTAL_WORK : BENCH_PER_THREAD_WORK,
        num_trials, num_threads, cpu_list, num_cpus, pin_cpus,
        prefer_hyper_threads);


    run_decls(&lock_list, run_all ? NULL : lock_names.ptrs,
              lock_names.n, &run_lock_test);

    if (stats) {
        uint32_t i, j;

        if (stats_csv) {
            enum { CSV_LINE_SIZE = 4096 };
            int64_t offset = 0, ret;
            char    csv_line[CSV_LINE_SIZE];
            die_assert((ret = snprintf(csv_line + offset,
                                       CSV_LINE_SIZE - offset, "%s",
                                       "lock")) >= 0);
            offset += ret;
            die_assert(offset < CSV_LINE_SIZE);
            for (i = 0; i <= num_trials; ++i) {
                die_assert(
                    (ret = snprintf(
                         csv_line + offset, CSV_LINE_SIZE - offset,
                         ",%s_%d,%s_%d,%s_%d,%s_%d,%s_%d,%s_%d,%s_%d",
                         "min", i, "max", i, "geomean", i, "stdev", i,
                         "median", i, "p5", i, "p95", i)) >= 0);
                offset += ret;
                die_assert(offset < CSV_LINE_SIZE);
            }

            printf("%s\n", csv_line);

            for (i = 0; i < stats_idx; ++i) {
                if (stats[i] == NULL) {
                    continue;
                }
                offset = 0;
                die_assert((ret = snprintf(csv_line + offset,
                                           CSV_LINE_SIZE - offset, "%s",
                                           stats_get_desc(stats[i]))) >=
                           0);
                offset += ret;
                die_assert(offset < CSV_LINE_SIZE);
                for (j = 0; j <= num_trials; ++j) {
                    die_assert(
                        (ret = snprintf(
                             csv_line + offset, CSV_LINE_SIZE - offset,
                             ",%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                             stats_get_min(stats[i] + j),
                             stats_get_max(stats[i] + j),
                             stats_get_geomean(stats[i] + j),
                             stats_get_stdev(stats[i] + j),
                             stats_get_median(stats[i] + j),
                             stats_get_percentile(stats[i] + j, 5),
                             stats_get_percentile(stats[i] + j, 95))) >=
                        0);
                    offset += ret;
                    die_assert(offset < CSV_LINE_SIZE);
                }
                printf("%s\n", csv_line);
                safe_sfree(stats[i],
                           (num_trials + 1) * sizeof(stats_result_t));
            }
        }
        else {
            printf(
                "Stats for lock benchmarks with (Trials = %u, Threads = %u)\n",
                num_trials, num_threads);


            for (i = 0; i < stats_idx; ++i) {
                if (num_trials > 1) {
                    stats_set_desc(stats[i] + (num_trials - 1),
                                   stats_get_desc(stats[i]));
                }

                stats_printf_arr(stdout, stats[i] + (num_trials - 1),
                                 STATS_P_desc | STATS_P_min |
                                     STATS_P_max | STATS_P_geomean |
                                     STATS_P_stdev,
                                 NULL, 0);
                printf("\n");

                safe_sfree(stats[i],
                           (num_trials + 1) * sizeof(stats_result_t));
            }
        }


        safe_sfree(stats, lock_list.ndecls * sizeof(stats_result_t));
    }

    run_params_destroy(&common_params);
    return 0;
}

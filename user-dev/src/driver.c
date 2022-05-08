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

static uint32_t outer_iter = 1000 * 1000;
static uint32_t cs_iter    = 1;
static uint32_t extra_iter = 0;

static int32_t      num_trials           = 1;
static int32_t      num_threads          = 1;
static char const * cpu_list             = NULL;
static int32_t      num_cpus             = -1;
static int32_t      pin_cpus             = 0;
static int32_t      prefer_hyper_threads = 0;

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
            "--all",
            0,
            &run_all,
            "Run all lock versions"),
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


static run_params_t     common_params;
static stats_result_t * stats;
static uint32_t         stats_idx;


static void
run_lock_test(func_decl_t const * lock) {
    die_assert(run(lock, &common_params, stats + stats_idx) == 0);
    if (stats) {
        stats_set_desc(stats + (stats_idx++), lock->name);
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


    if (do_list || (!run_all && lock_names.n == 0)) {
        list_decls(&lock_list);
        return 0;
    }
    else if (do_bench) {
        stats = (stats_result_t *)safe_calloc(lock_list.ndecls,
                                              sizeof(stats_result_t));
    }
    run_params_init(&common_params, outer_iter, cs_iter, extra_iter,
                    num_trials, num_threads, cpu_list, num_cpus,
                    pin_cpus, prefer_hyper_threads);


    run_decls(&lock_list, run_all ? NULL : lock_names.ptrs,
              lock_names.n, &run_lock_test);

    if (stats) {
        uint32_t i;
        printf(
            "Stats for lock benchmarks with (Trials = %u, Threads = %u)\n",
            num_trials, num_threads);
        for (i = 0; i < stats_idx; ++i) {
            stats_printf_arr(stdout, stats + i,
                             STATS_P_desc | STATS_P_min | STATS_P_max |
                                 STATS_P_geomean | STATS_P_stdev,
                             NULL, 0);
            printf("\n");
        }


        safe_sfree(stats, lock_list.ndecls * sizeof(stats_result_t));
    }

    run_params_destroy(&common_params);
    return 0;
}

#include "bench/bench-common.h"
#include "bench/benchmarks.h"

#include "util/arg.h"
#include "util/error-util.h"
#include "util/func-decl-utils.h"
#include "util/macro.h"
#include "util/sched-util.h"
#include "util/verbosity.h"

static int32_t    verbosity       = 0;
static int32_t    list_all        = 0;
static int32_t    run_all         = 0;
static uint32_t   trials          = 0;
static int32_t    cpu             = -1;
static arg_rest_t benchmark_names = INIT_ARG_REST_T;

static ArgOption args[] = {
    /* ADD_ARG(Kind, Method, name, reqd, variable, help) */
    ADD_ARG(KindOption, Integer, "-v", 0, &verbosity, "Turn on verbosity"),
    ADD_ARG(KindOption, Set, "--list", 0, &list_all, "List all benchmarks"),
    ADD_ARG(KindOption, Set, "--all", 0, &run_all, "Run all benchmarks"),
    ADD_ARG(KindOption, Integer, "--cpu", 0, &cpu, "Set CPU (-1 to skip)"),
    ADD_ARG(KindOption,
            Integer,
            "-N",
            0,
            &trials,
            "Trials (0 will use default)"),
    ADD_ARG(KindHelp, Help, "-h", 0, NULL, ""),
    ADD_ARG(KindRest,
            Rest,
            "",
            0,
            &benchmark_names,
            "Benchmarks to run. Just include at end"),
    ADD_ARG(KindEnd, EndOptions, "", 0, NULL, ""),
};


static ArgDefs argp = { args, "Benchmark Driver", NULL, NULL };


static void
run_benchmark(const func_decl_t * benchmark) {

    warn_assert(
        !benchmark->bench_func(
            /* Ideally replaced with NOLINT(performance-no-int-to-ptr) */
            NOANALYZE(CAST(void *, CAST(uint64_t, trials)), NULL)),
        "Issue running: %s\n", benchmark->name);
}

int
main(int argc, char ** argv) {
    die_assert(!doParse(&argp, argc, argv), "Error parsing arguments\n");
    set_verbosity(verbosity);
    if (list_all || (!run_all && benchmark_names.n == 0)) {
        list_decls(&benchmarks);
    }
    else {
        proc_setcpu_and_wait(cpu);
        run_decls(&benchmarks, run_all ? NULL : benchmark_names.ptrs,
                  benchmark_names.n, &run_benchmark);
    }
    return 0;
}

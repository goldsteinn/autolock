#include "test/test-common.h"
#include "test/tests.h"

#include "util/arg.h"
#include "util/error-util.h"
#include "util/verbosity.h"

#include "util/func-decl-utils.h"

static int32_t    verbosity  = 0;
static int32_t    list_all   = 0;
static int32_t    run_all    = 0;
static arg_rest_t test_names = INIT_ARG_REST_T;

static ArgOption args[] = {
    /* ADD_ARG(Kind, Method, name, reqd, variable, help) */
    ADD_ARG(KindOption, Integer, "-v", 0, &verbosity, "Turn on verbosity"),
    ADD_ARG(KindOption, Set, "--list", 0, &list_all, "List all tests"),
    ADD_ARG(KindOption, Set, "--all", 0, &run_all, "Run all tests"),
    ADD_ARG(KindHelp, Help, "-h", 0, NULL, ""),
    ADD_ARG(KindRest,
            Rest,
            "",
            0,
            &test_names,
            "Tests to run. Just include at end"),
    ADD_ARG(KindEnd, EndOptions, "", 0, NULL, "")
};


static ArgDefs argp = { args, "Test Driver", NULL, NULL };


static void
run_test(const func_decl_t * test) {
    const char * res = "PASSED";
    printf("Running - %-24s ...", test->name);
    if (test->test_func()) {
        res = "FAILED";
    }
    printf("\rRunning - %-24s -> %s\n", test->name, res);
    fflush(stdout);
}

int
main(int argc, char ** argv) {
    die_assert(!doParse(&argp, argc, argv), "Error parsing arguments\n");
    set_verbosity(verbosity);

    if (list_all || (!run_all && test_names.n == 0)) {
        list_decls(&tests);
    }
    else {
        run_decls(&tests, run_all ? NULL : test_names.ptrs, test_names.n,
                  &run_test);
    }

    return 0;
}

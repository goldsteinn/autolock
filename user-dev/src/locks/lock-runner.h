#ifndef _SRC__LOCKS__RUNNER_H_
#define _SRC__LOCKS__RUNNER_H_

#include "lib/stats/stats.h"
#include "locks/lock-run-params.h"
#include "util/func-decl-types.h"

/* Run benchmark for a given lock impl (to_run) with a set of params
 * (params) and optional result (result_out). */
int32_t run(func_decl_t const * to_run,
            run_params_t *      params,
            stats_result_t *    result_out);
#endif

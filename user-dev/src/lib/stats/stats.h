#ifndef _SRC__LIB__STATS__STATS_H_
#define _SRC__LIB__STATS__STATS_H_

#include "util/common.h"
#include "util/error-util.h"
#include "util/macro.h"
#include "util/types.h"

/* Add any additional percentiles worth collecting here. Must be sorted. */
#define STATS_SUPPORTED_PERCENTILES                                            \
    0, 1, 5, 10, 25, 33, 50, 66, 75, 90, 95, 99, 100

static const uint8_t I_percentiles[] = { STATS_SUPPORTED_PERCENTILES };
const_assert(sizeof(I_percentiles[0]) == 1);

enum {
    STATS_P_desc         = (1 << 0),
    STATS_P_min          = (1 << 1),
    STATS_P_max          = (1 << 2),
    STATS_P_median       = (1 << 3),
    STATS_P_mean         = (1 << 4),
    STATS_P_geomean      = (1 << 5),
    STATS_P_stdev        = (1 << 6),
    STATS_P_variance     = (1 << 7),
    STATS_P_percentiles  = (1 << 8),
    STATS_P_csv_hdr      = (1 << 9),
    STATS_P_csv_hdr_only = (1 << 10),
    STATS_P_all          = (-1U) ^ (STATS_P_csv_hdr | STATS_P_csv_hdr_only)
};

typedef struct stats_result {
    char const * desc;
    uint64_t     has_stats;
    double       min;
    double       max;
    double       median;
    double       mean;
    double       geomean;
    double       stdev;
    double       variance;

    double percentiles[sizeof(I_percentiles)];
} stats_result_t;

static uint32_t
num_percentiles() {
    return sizeof(I_percentiles);
}
static uint32_t
idx_to_percentile(uint32_t idx) {
    die_assert(idx < num_percentiles());
    return I_percentiles[idx];
}

static NONNULL(1) void stats_set_desc(stats_result_t * restrict stats,
                                      char const * restrict desc) {
    warn_assert(stats->desc == NULL,
                "stats overwritting existing description\n");
    stats->desc = desc;
}

static NONNULL(1) char const * stats_get_desc(stats_result_t const * stats) {
    return stats->desc;
}

static NONNULL(1) double stats_get_min(stats_result_t const * stats) {
    return stats->min;
}

static NONNULL(1) double stats_get_max(stats_result_t const * stats) {
    return stats->max;
}

static NONNULL(1) double stats_get_median(stats_result_t const * stats) {
    return stats->median;
}

static NONNULL(1) double stats_get_mean(stats_result_t const * stats) {
    return stats->mean;
}


static NONNULL(1) double stats_get_geomean(stats_result_t const * stats) {
    return stats->geomean;
}

static NONNULL(1) double stats_get_stdev(stats_result_t const * stats) {
    return stats->stdev;
}

static NONNULL(1) double stats_get_variance(stats_result_t const * stats) {
    return stats->variance;
}


static uint32_t
I_stats_get_percentile_idx(uint32_t percentile) {
    uint32_t i, v;
    for (i = 0; i < num_percentiles(); ++i) {
        v = idx_to_percentile(i);
        if (percentile <= v) {
            if (percentile < v) {
                i = num_percentiles();
            }
            break;
        }
    }
    return i;
}

static NONNULL(1) double stats_get_percentile(stats_result_t const * stats,
                                              uint32_t percentile) {
    uint32_t idx;
    idx = I_stats_get_percentile_idx(percentile);
    die_assert(idx < num_percentiles(), "Unknown percentile: %u\n", percentile);
    return stats->percentiles[idx];
}


NONNULL(1, 2)
void I_gen_stats(stats_result_t * restrict stats,
                 uint8_t const * restrict mem,
                 uint64_t nelem,
                 uint32_t elem_sz,
                 /* Take is_unsigned and is_fp_based so void * data can just be
                    intepretted as uint64_t. */
                 uint32_t is_unsigned,
                 uint32_t is_fp_based);

#define gen_stats(stats, arr, arrsize)                                         \
    I_gen_stats(stats, CAST(uint8_t const *, arr), arrsize, sizeof(arr[0]),    \
                IS_UNSIGNED(arr[0]), IS_FLOAT_BASE(arr[0]))


NONNULL(1, 3)
uint64_t stats_prints_arr(char *                 buf,
                          uint64_t               buflen,
                          stats_result_t const * stats,
                          uint32_t               to_print,
                          uint32_t const *       percentiles,
                          uint32_t               npercentiles);

NONNULL(2)
uint64_t stats_printf_arr(FILE *                 fp,
                          stats_result_t const * stats,
                          uint32_t               to_print,
                          uint32_t const *       percentiles,
                          uint32_t               npercentiles);


NONNULL(1, 3)
uint64_t stats_prints_csv_arr(char *                 buf,
                              uint64_t               buflen,
                              stats_result_t const * stats,
                              uint32_t               to_print,
                              uint32_t const *       percentiles,
                              uint32_t               npercentiles);

NONNULL(2)
uint64_t stats_printf_csv_arr(FILE *                 fp,
                              stats_result_t const * stats,
                              uint32_t               to_print,
                              uint32_t const *       percentiles,
                              uint32_t               npercentiles);


NONNULL(1, 3, 4)
uint64_t stats_prints_cmp_arr(char *                 buf,
                              uint64_t               buflen,
                              stats_result_t const * statsA,
                              stats_result_t const * statsB,
                              uint32_t               to_print,
                              uint32_t const *       percentiles,
                              uint32_t               npercentiles);

NONNULL(2, 3)
uint64_t stats_printf_cmp_arr(FILE *                 fp,
                              stats_result_t const * statsA,
                              stats_result_t const * statsB,
                              uint32_t               to_print,
                              uint32_t const *       percentiles,
                              uint32_t               npercentiles);


#define stats_prints(buf, buflen, stats, to_print, ...)                        \
    stats_prints_arr(buf, buflen, stats, to_print,                             \
                     ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))

#define stats_printf(fp, stats, to_print, ...)                                 \
    stats_printf_arr(fp, stats, to_print,                                      \
                     ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))


#define stats_prints_csv(buf, buflen, stats, to_print, ...)                    \
    stats_prints_csv_arr(buf, buflen, stats, to_print,                         \
                         ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))

#define stats_printf_csv(fp, stats, to_print, ...)                             \
    stats_printf_csv_arr(fp, stats, to_print,                                  \
                         ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))


#define stats_prints_cmp(buf, buflen, statsA, statsB, to_print, ...)           \
    stats_prints_cmp_arr(buf, buflen, statsA, statsB, to_print,                \
                         ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))

#define stats_printf_cmp(fp, statsA, statsB, to_print, ...)                    \
    stats_printf_cmp_arr(fp, statsA, statsB, to_print,                         \
                         ARR_ARG_T(uint32_t, DEPAREN(__VA_ARGS__)))


#endif

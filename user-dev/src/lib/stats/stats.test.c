#include "lib/stats/stats.h"

#include "util/macro.h"

#include "test/test-common.h"


#define stats_test_name(T) CAT(stats_test_, T)
#define run_test(T)        test_assert(stats_test_name(T)() == 0)
#define make_stats_test(T)                                                        \
    static int32_t stats_test_name(T)() {                                         \
        if (is_same_type(T, long_double)) {                                       \
            return 0;                                                             \
        }                                                                         \
        typedef struct stats_tests { /* NOLINT(altera-struct-pack-align) */       \
            const stats_result_t expec;                                           \
            uint64_t             arr_size;                                        \
            T                    arr[256];                                        \
        } stats_tests_t;                                                          \
                                                                                  \
        static const stats_tests_t tests[] = {                                    \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 100.000000,                            \
                         .median      = 50.500000,                             \
                         .mean        = 50.500000,                             \
                         .geomean     = 37.992689,                             \
                         .stdev       = 29.011492,                             \
                         .variance    = 841.666667,                            \
                         .percentiles = { 1.0, 1.99, 5.95, 10.9, 25.75, 33.67, \
                                          50.5, 66.34, 75.25,                  \
                                          90.10000000000001, 95.05, 99.01,     \
                                          100.0 } },                           \
              .arr_size = 100,                                                    \
              .arr      = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,   \
                       14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,   \
                       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,   \
                       40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,   \
                       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,   \
                       66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,   \
                       79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,   \
                       92, 93, 94, 95, 96, 97, 98, 99, 100 } },                   \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 99.000000,                             \
                         .median      = 50.000000,                             \
                         .mean        = 50.000000,                             \
                         .geomean     = 37.623100,                             \
                         .stdev       = 28.722813,                             \
                         .variance    = 825.000000,                            \
                         .percentiles = { 1.0, 1.98, 5.9, 10.8, 25.5, 33.34,   \
                                          50.0, 65.68, 74.5, 89.2, 94.1,       \
                                          98.02, 99.0 } },                     \
              .arr_size = 99,                                                     \
              .arr      = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,   \
                       14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,   \
                       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,   \
                       40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,   \
                       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,   \
                       66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,   \
                       79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,   \
                       92, 93, 94, 95, 96, 97, 98, 99 } },                        \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 14.000000,                             \
                         .median      = 7.500000,                              \
                         .mean        = 7.500000,                              \
                         .geomean     = 6.045855,                              \
                         .stdev       = 4.183300,                              \
                         .variance    = 17.500000,                             \
                         .percentiles = { 1.0, 1.13, 1.65, 2.3, 4.25, 5.29,    \
                                          7.5, 9.58, 10.75,                    \
                                          12.700000000000001, 13.35, 13.87,    \
                                          14.0 } },                            \
              .arr_size = 14,                                                     \
              .arr      = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 } },         \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 100.000000,                            \
                         .median      = 50.500000,                             \
                         .mean        = 50.500000,                             \
                         .geomean     = 37.992689,                             \
                         .stdev       = 29.011492,                             \
                         .variance    = 841.666667,                            \
                         .percentiles = { 1.0, 1.99, 5.95, 10.9, 25.75, 33.67, \
                                          50.5, 66.34, 75.25,                  \
                                          90.10000000000001, 95.05, 99.01,     \
                                          100.0 } },                           \
              .arr_size = 100,                                                    \
              .arr      = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,   \
                       14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,   \
                       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,   \
                       40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,   \
                       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,   \
                       66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,   \
                       79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,   \
                       92, 93, 94, 95, 96, 97, 98, 99, 100 } },                   \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 99.000000,                             \
                         .median      = 50.000000,                             \
                         .mean        = 50.000000,                             \
                         .geomean     = 37.623100,                             \
                         .stdev       = 28.722813,                             \
                         .variance    = 825.000000,                            \
                         .percentiles = { 1.0, 1.98, 5.9, 10.8, 25.5, 33.34,   \
                                          50.0, 65.68, 74.5, 89.2, 94.1,       \
                                          98.02, 99.0 } },                     \
              .arr_size = 99,                                                     \
              .arr      = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,   \
                       14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,   \
                       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,   \
                       40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,   \
                       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,   \
                       66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,   \
                       79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,   \
                       92, 93, 94, 95, 96, 97, 98, 99 } },                        \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 9.000000,                              \
                         .median      = 5.000000,                              \
                         .mean        = 5.000000,                              \
                         .geomean     = 4.147166,                              \
                         .stdev       = 2.738613,                              \
                         .variance    = 7.500000,                              \
                         .percentiles = { 1.0, 1.08, 1.4, 1.8, 3.0, 3.64, 5.0, \
                                          6.28, 7.0, 8.2, 8.6, 8.92, 9.0 } },  \
              .arr_size = 9,                                                      \
              .arr      = { 1, 2, 3, 4, 5, 6, 7, 8, 9 } },                             \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 14.000000,                             \
                         .median      = 7.500000,                              \
                         .mean        = 7.500000,                              \
                         .geomean     = 6.045855,                              \
                         .stdev       = 4.183300,                              \
                         .variance    = 17.500000,                             \
                         .percentiles = { 1.0, 1.13, 1.65, 2.3, 4.25, 5.29,    \
                                          7.5, 9.58, 10.75,                    \
                                          12.700000000000001, 13.35, 13.87,    \
                                          14.0 } },                            \
              .arr_size = 14,                                                     \
              .arr      = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 } },         \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 1.000000,                              \
                         .max         = 10.000000,                             \
                         .median      = 5.500000,                              \
                         .mean        = 5.500000,                              \
                         .geomean     = 4.528729,                              \
                         .stdev       = 3.027650,                              \
                         .variance    = 9.166667,                              \
                         .percentiles = { 1.0, 1.09, 1.45, 1.9, 3.25, 3.97,    \
                                          5.5, 6.94, 7.75, 9.1,                \
                                          9.549999999999999, 9.91, 10.0 } },   \
              .arr_size = 10,                                                     \
              .arr      = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },                         \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 10.000000,                             \
                         .max         = 11.000000,                             \
                         .median      = 10.500000,                             \
                         .mean        = 10.500000,                             \
                         .geomean     = 10.488088,                             \
                         .stdev       = 0.707107,                              \
                         .variance    = 0.500000,                              \
                         .percentiles = { 10.0, 10.01, 10.05, 10.1, 10.25,     \
                                          10.33, 10.5, 10.66, 10.75, 10.9,     \
                                          10.95, 10.99, 11.0 } },              \
              .arr_size = 2,                                                      \
              .arr      = { 10, 11 } },                                                \
            { .expec    = { .desc        = NULL,                                  \
                         .has_stats   = 1,                                     \
                         .min         = 10.000000,                             \
                         .max         = 10.000000,                             \
                         .median      = 10.000000,                             \
                         .mean        = 10.000000,                             \
                         .geomean     = 10.000000,                             \
                         .stdev       = 0.000000,                              \
                         .variance    = 0.000000,                              \
                         .percentiles = { 10.0, 10.0, 10.0, 10.0, 10.0, 10.0,  \
                                          10.0, 10.0, 10.0, 10.0, 10.0, 10.0,  \
                                          10.0 } },                            \
              .arr_size = 1,                                                      \
              .arr      = { 10 } }                                                \
        };                                                                        \
        uint32_t       ntests = sizeof(tests) / sizeof(tests[0]);                 \
        uint32_t       i, j;                                                      \
        stats_result_t res;                                                       \
        enum { DEFAULT_BUFLEN = 2048 };                                           \
        char buf[DEFAULT_BUFLEN];                                                 \
        for (i = 0; i < ntests; ++i) {                                            \
            gen_stats(&res, tests[i].arr, tests[i].arr_size);                     \
                                                                                  \
            test_assert(float_eq(stats_get_min(&res),                             \
                                 stats_get_min(&(tests[i].expec))),               \
                        "stats_get_min(%u): %lf != %lf\n", i,                     \
                        stats_get_min(&res),                                      \
                        stats_get_min(&(tests[i].expec)));                        \
            test_assert(float_eq(stats_get_max(&res),                             \
                                 stats_get_max(&(tests[i].expec))),               \
                        "stats_get_max(%u): %lf != %lf\n", i,                     \
                        stats_get_max(&res),                                      \
                        stats_get_max(&(tests[i].expec)));                        \
            test_assert(float_eq(stats_get_median(&res),                          \
                                 stats_get_median(&(tests[i].expec))),            \
                        "stats_get_median(%u): %lf != %lf\n", i,                  \
                        stats_get_median(&res),                                   \
                        stats_get_median(&(tests[i].expec)));                     \
            test_assert(float_eq(stats_get_mean(&res),                            \
                                 stats_get_mean(&(tests[i].expec))),              \
                        "stats_get_mean(%u): %lf != %lf\n", i,                    \
                        stats_get_mean(&res),                                     \
                        stats_get_mean(&(tests[i].expec)));                       \
            test_assert(float_eq(stats_get_geomean(&res),                         \
                                 stats_get_geomean(&(tests[i].expec))),           \
                        "stats_get_geomean(%u): %lf != %lf\n", i,                 \
                        stats_get_geomean(&res),                                  \
                        stats_get_geomean(&(tests[i].expec)));                    \
            test_assert(float_eq(stats_get_stdev(&res),                           \
                                 stats_get_stdev(&(tests[i].expec))),             \
                        "stats_get_stdev(%u): %lf != %lf\n", i,                   \
                        stats_get_stdev(&res),                                    \
                        stats_get_stdev(&(tests[i].expec)));                      \
            test_assert(float_eq(stats_get_variance(&res),                        \
                                 stats_get_variance(&(tests[i].expec))),          \
                        "stats_get_variance(%u): %lf != %lf\n", i,                \
                        stats_get_variance(&res),                                 \
                        stats_get_variance(&(tests[i].expec)));                   \
                                                                                  \
            for (j = 0; j < num_percentiles(); ++j) {                             \
                test_assert(                                                      \
                    float_eq(stats_get_percentile(&res, idx_to_percentile(j)),    \
                             stats_get_percentile(&(tests[i].expec),              \
                                                  idx_to_percentile(j))),         \
                    "stats_get_percentile(%u): %lf != %lf\n", i,                  \
                    stats_get_percentile(&res, idx_to_percentile(j)),             \
                    stats_get_percentile(&(tests[i].expec),                       \
                                         idx_to_percentile(j)));                  \
            }                                                                     \
            /* Just to test the interfaces. */                                    \
            stats_prints_csv(buf, DEFAULT_BUFLEN, &res, STATS_P_all,              \
                             STATS_SUPPORTED_PERCENTILES);                        \
            stats_prints(buf, DEFAULT_BUFLEN, &res, STATS_P_all,                  \
                         STATS_SUPPORTED_PERCENTILES);                            \
            stats_prints_cmp(buf, DEFAULT_BUFLEN, &res, &(tests[i].expec),        \
                             STATS_P_all, STATS_SUPPORTED_PERCENTILES);           \
        }                                                                         \
        return 0;                                                                 \
    }


APPLY(make_stats_test, ;, ALL_TYPE_NAMES);

int32_t
test_stats() {
    set_max_ulp(-1);
    APPLY(run_test, ;, ALL_TYPE_NAMES);
    return 0;
}

#include <math.h>

#include "lib/algo/sort.h"
#include "lib/stats/stats.h"

#include "util/file-util.h"
#include "util/inline-math.h"
#include "util/memory-util.h"


#define I_cvt_to_dbl(T, mem)                                                   \
    ({                                                                         \
        T I_tmp_val_;                                                          \
        __builtin_memcpy(&I_tmp_val_, mem, sizeof(I_tmp_val_));                \
        CAST(double, I_tmp_val_);                                              \
    })

static PURE_FUNC double
cvt_to_dbl(uint8_t const * mem,
           uint32_t        elem_sz,
           uint32_t        is_unsigned,
           uint32_t        is_fp_based) {
    switch (elem_sz) {
        case 1: {
            die_assert(!is_fp_based);
            if (is_unsigned) {
                return I_cvt_to_dbl(uint8_t, mem);
            }
            else {
                return I_cvt_to_dbl(int8_t, mem);
            }
        }
        case 2: {
            die_assert(!is_fp_based);
            if (is_unsigned) {
                return I_cvt_to_dbl(uint16_t, mem);
            }
            else {
                return I_cvt_to_dbl(int16_t, mem);
            }
        }
        case 4: {
            if (is_unsigned) {
                die_assert(!is_fp_based);
                return I_cvt_to_dbl(uint32_t, mem);
            }
            else {
                if (is_fp_based) {
                    return I_cvt_to_dbl(float, mem);
                }
                else {
                    return I_cvt_to_dbl(int32_t, mem);
                }
            }
        }
        case 8: {
            if (is_unsigned) {
                die_assert(!is_fp_based);
                return I_cvt_to_dbl(uint64_t, mem);
            }
            else {
                if (is_fp_based) {
                    return I_cvt_to_dbl(double, mem);
                }
                else {
                    return I_cvt_to_dbl(int64_t, mem);
                }
            }
        }
        default:
            die("Unknown type size");
    };
}
static PURE_FUNC double
calc_median(double const * dbl_data, uint64_t nelem) {
    if (nelem % 2) {
        return dbl_data[nelem / 2];
    }
    else {
        return (dbl_data[nelem / 2] + dbl_data[(nelem / 2) - 1]) / 2.0;
    }
}

static PURE_FUNC double
calc_mean(double const * dbl_data, uint64_t nelem) {
    uint64_t i;
    double   sum = 0.0;

    for (i = 0; i < nelem; ++i) {
        sum += dbl_data[i];
    }
    return sum / CAST(double, nelem);
}

static PURE_FUNC double
calc_geomean(double const * dbl_data, uint64_t nelem) {
#ifdef WITH_MATH
    double   sum = 0.0;
    uint64_t i;
    for (i = 0; i < nelem; ++i) {
        sum += log(dbl_data[i]);
    }
    sum /= CAST(double, nelem);
    return exp(sum);
#else
    (void)(dbl_data);
    (void)(nelem);
    return 0.0;
#endif
}

static PURE_FUNC double
calc_squared_sum(double const * dbl_data, uint64_t nelem) {
    uint64_t i;
    double   mean, squared_sum;
    squared_sum = 0;
    mean        = calc_mean(dbl_data, nelem);
    for (i = 0; i < nelem; ++i) {
        double tmp = dbl_data[i] - mean;
        squared_sum += (tmp * tmp);
    }
    return squared_sum;
}

static PURE_FUNC double
calc_stdev(double const * dbl_data, uint64_t nelem) {
    if (nelem <= 1) {
        return 0.0;
    }
#ifdef WITH_MATH
    return sqrt(calc_squared_sum(dbl_data, nelem) / CAST(double, (nelem - 1)));
#else
    (void)(dbl_data);
    (void)(nelem);
    return 0.0;
#endif
}

static PURE_FUNC double
calc_variance(double const * dbl_data, uint64_t nelem) {
    if (nelem <= 1) {
        return 0.0;
    }
    return calc_squared_sum(dbl_data, nelem) / CAST(double, nelem - 1);
}

static void
calc_percentiles(stats_result_t * restrict stats,
                 double const * dbl_data,
                 uint64_t       nelem) {
    uint32_t i;
    uint64_t idx;
    double   dbl_idx;
    double   nelem_mul = CAST(double, nelem - 1) / CAST(double, 100.0);
    for (i = 0; i < num_percentiles(); ++i) {
        dbl_idx = CAST(double, I_percentiles[i]) * nelem_mul;
        idx     = CAST(uint64_t, dbl_idx);
        /* No weighted average needed on exact match or if idx + 1 is out of
         * bounds. */
        if (dbl_idx == CAST(double, idx) || (idx + 1 >= nelem)) {
            stats->percentiles[i] = dbl_data[idx];
        }
        else {
            double val_lo    = dbl_data[idx];
            double val_hi    = dbl_data[idx + 1];
            double remainder = dbl_idx - CAST(double, idx);

            /* Weighted average depending on how close we are to each index. */
            val_lo *= (1 - remainder);
            val_hi *= remainder;

            stats->percentiles[i] = val_lo + val_hi;
        }
    }
}

static void
calc_stats(stats_result_t * restrict stats,
           double * restrict dbl_data,
           uint64_t nelem) {
    die_assert(nelem > 0);

    generic_sort(dbl_data, nelem);

    stats->min      = dbl_data[0];
    stats->max      = dbl_data[nelem - 1];
    stats->median   = calc_median(dbl_data, nelem);
    stats->mean     = calc_mean(dbl_data, nelem);
    stats->geomean  = calc_geomean(dbl_data, nelem);
    stats->stdev    = calc_stdev(dbl_data, nelem);
    stats->variance = calc_variance(dbl_data, nelem);

    calc_percentiles(stats, dbl_data, nelem);
}

void
I_gen_stats(stats_result_t * restrict stats,
            uint8_t const * restrict mem,
            uint64_t nelem,
            uint32_t elem_sz,
            uint32_t is_unsigned,
            uint32_t is_fp_based) {
    double * dbl_data;
    uint64_t i;

    __builtin_memset(stats, 0, sizeof(*stats));
    stats->has_stats = 1;
    if (nelem == 0) {
        warn("taking stats with zero elements\n");
        return;
    }


    die_assert(elem_sz <= sizeof(double));
    die_assert((!is_fp_based) || (!is_unsigned));


    dbl_data = (double *)safe_calloc(nelem, sizeof(double));


    for (i = 0; i < nelem; ++i) {
        dbl_data[i] = cvt_to_dbl(mem, elem_sz, is_unsigned, is_fp_based);
        mem += elem_sz;
    }

    calc_stats(stats, dbl_data, nelem);

    safe_sfree(dbl_data, nelem * sizeof(double));
}


static int32_t
stats_print_check(char const *           buf,
                  stats_result_t const * statsA,
                  stats_result_t const * statsB,
                  uint32_t *             to_print) {
    die_assert(buf != NULL);
    die_assert(statsA);

    if (!statsA->has_stats) {
        warn("No statsA where collected before print call\n");
        return 1;
    }

    if (statsB && !statsB->has_stats) {
        warn("No statsB where collected before print call\n");
        return 1;
    }

    /* Default zero to STATS_P_all. */
    *to_print = *to_print ? *to_print : STATS_P_all;
    return 0;
}


uint64_t
stats_prints_cmp_arr(char *                 buf,
                     uint64_t               buflen,
                     stats_result_t const * statsA,
                     stats_result_t const * statsB,
                     uint32_t               to_print,
                     uint32_t const *       percentiles,
                     uint32_t               npercentiles) {

#define STATS_DESC_FMT_STR       "%-16s: %16s vs %16s\n"
#define STATS_FIELD_FMT_STR      "%-16s: %16.3lf vs %16.3lf -> %6.3lf\n"
#define STATS_PERCENTILE_FMT_STR "%-11s(%3d): %16.3lf vs %16.3lf -> %6.3lf\n"


#define STATS_DO_PRINT(field, fmt)                                             \
    nbytes_req += snprintf(                                                    \
        buf + nbytes_req, buflen - nbytes_req, fmt, V_TO_STR(field),           \
        CAT(stats_get_, field)(statsA), CAT(stats_get_, field)(statsB),        \
        CAT(stats_get_, field)(statsA) / CAT(stats_get_, field)(statsB));      \
    die_assert(nbytes_req < buflen, "Buffer to small to format stats!\n");

#define STATS_CHECK_PRINT(field)                                               \
    if (CAT(STATS_P_, field) & to_print) {                                     \
        STATS_DO_PRINT(field, STATS_FIELD_FMT_STR);                            \
    }

    uint64_t nbytes_req = 0;
    if (stats_print_check(buf, statsA, statsB, &to_print)) {
        return 0;
    }

    if (STATS_P_desc & to_print) {
        nbytes_req += snprintf(buf + nbytes_req, buflen - nbytes_req,
                               STATS_DESC_FMT_STR, "Comparing",
                               stats_get_desc(statsA), stats_get_desc(statsB));
        die_assert(nbytes_req < buflen, "Buffer to small to format stats!\n");
    }

    STATS_CHECK_PRINT(min);
    STATS_CHECK_PRINT(max);
    STATS_CHECK_PRINT(median);
    STATS_CHECK_PRINT(mean);
    STATS_CHECK_PRINT(geomean);
    STATS_CHECK_PRINT(stdev);
    STATS_CHECK_PRINT(variance);

    if (STATS_P_percentiles & to_print) {
        uint32_t i, percentile;
        for (i = 0; i < npercentiles; ++i) {
            percentile = percentiles[i];
            nbytes_req +=
                snprintf(buf + nbytes_req, buflen - nbytes_req,
                         STATS_PERCENTILE_FMT_STR, "percentile", percentile,
                         stats_get_percentile(statsA, percentile),
                         stats_get_percentile(statsB, percentile),
                         stats_get_percentile(statsA, percentile) /
                             stats_get_percentile(statsB, percentile));

            die_assert(nbytes_req < buflen,
                       "Buffer to small to format stats!\n");
        }
    }

    return nbytes_req;


#undef STATS_DESC_FMT_STR
#undef STATS_FIELD_FMT_STR
#undef STATS_PERCENTILE_FMT_STR
#undef STATS_DO_PRINT
#undef STATS_CHECK_PRINT
}

uint64_t
stats_prints_arr(char *                 buf,
                 uint64_t               buflen,
                 stats_result_t const * stats,
                 uint32_t               to_print,
                 uint32_t const *       percentiles,
                 uint32_t               npercentiles) {

#define STATS_DESC_FMT_STR       "%-16s: %10s\n"
#define STATS_FIELD_FMT_STR      "%-16s: %10.3lf\n"
#define STATS_PERCENTILE_FMT_STR "%-11s(%3d): %10.3lf\n"

#define STATS_DO_PRINT(field, fmt)                                             \
    nbytes_req += snprintf(buf + nbytes_req, buflen - nbytes_req, fmt,         \
                           V_TO_STR(field), CAT(stats_get_, field)(stats));    \
    die_assert(nbytes_req < buflen, "Buffer to small to format stats!\n");

#define STATS_CHECK_PRINT(field)                                               \
    if (CAT(STATS_P_, field) & to_print) {                                     \
        STATS_DO_PRINT(field, STATS_FIELD_FMT_STR);                            \
    }


    uint64_t nbytes_req = 0;
    if (stats_print_check(buf, stats, NULL, &to_print)) {
        return 0;
    }

    if (STATS_P_desc & to_print) {
        STATS_DO_PRINT(desc, STATS_DESC_FMT_STR);
    }

    STATS_CHECK_PRINT(min);
    STATS_CHECK_PRINT(max);
    STATS_CHECK_PRINT(median);
    STATS_CHECK_PRINT(mean);
    STATS_CHECK_PRINT(geomean);
    STATS_CHECK_PRINT(stdev);
    STATS_CHECK_PRINT(variance);

    if (STATS_P_percentiles & to_print) {
        uint32_t i, percentile;
        for (i = 0; i < npercentiles; ++i) {
            percentile = percentiles[i];
            nbytes_req +=
                snprintf(buf + nbytes_req, buflen - nbytes_req,
                         STATS_PERCENTILE_FMT_STR, "percentile", percentile,
                         stats_get_percentile(stats, percentile));

            die_assert(nbytes_req < buflen,
                       "Buffer to small to format stats!\n");
        }
    }

    return nbytes_req;

#undef STATS_DESC_FMT_STR
#undef STATS_FIELD_FMT_STR
#undef STATS_PERCENTILE_FMT_STR
#undef STATS_DO_PRINT
#undef STATS_CHECK_PRINT
}


uint64_t
stats_prints_csv_arr(char *                 buf,
                     uint64_t               buflen,
                     stats_result_t const * stats,
                     uint32_t               to_print,
                     uint32_t const *       percentiles,
                     uint32_t               npercentiles) {

#define STATS_DO_PRINT(fmt, ...)                                               \
    nbytes_req +=                                                              \
        snprintf(buf + nbytes_req, buflen - nbytes_req, "%s" fmt,              \
                 (nbytes_req && (buf[nbytes_req - 1] != '\n')) ? "," : "",     \
                 __VA_ARGS__);                                                 \
    die_assert(nbytes_req < buflen, "Buffer to small to format stats!\n");

#define STATS_CHECK_PRINT(field)                                               \
    if (CAT(STATS_P_, field) & to_print) {                                     \
        if (i == 0 && STATS_P_csv_hdr & to_print) {                            \
            STATS_DO_PRINT("%s", V_TO_STR(field));                             \
        }                                                                      \
        else if (i) {                                                          \
            STATS_DO_PRINT("%lf", CAT(stats_get_, field)(stats));              \
        }                                                                      \
    }


    uint64_t nbytes_req = 0;
    uint32_t i;
    if (stats_print_check(buf, stats, NULL, &to_print)) {
        return 0;
    }
    for (i = 0; i < 2; ++i) {
        if (i == 0 && STATS_P_csv_hdr & to_print) {
            STATS_DO_PRINT("%s", "desc");
        }
        else if (i) {
            STATS_DO_PRINT("%s", stats_get_desc(stats));
        }

        STATS_CHECK_PRINT(min);
        STATS_CHECK_PRINT(max);
        STATS_CHECK_PRINT(median);
        STATS_CHECK_PRINT(mean);
        STATS_CHECK_PRINT(geomean);
        STATS_CHECK_PRINT(stdev);
        STATS_CHECK_PRINT(variance);

        if (STATS_P_percentiles & to_print) {
            uint32_t j, percentile;
            for (j = 0; j < npercentiles; ++j) {
                percentile = percentiles[j];
                if (i == 0 && STATS_P_csv_hdr & to_print) {
                    STATS_DO_PRINT("%s(%d)", "percentile", percentile);
                }
                else if (i) {
                    STATS_DO_PRINT("%lf",
                                   stats_get_percentile(stats, percentile));
                }
            }
        }
        if (STATS_P_csv_hdr_only & to_print) {
            break;
        }
        die_assert(nbytes_req < buflen);
        buf[nbytes_req++] = '\n';
    }
    die_assert(nbytes_req < buflen);
    buf[nbytes_req++] = '\0';
    return nbytes_req;

#undef STATS_DO_PRINT
#undef STATS_CHECK_PRINT
}


uint64_t
stats_printf_cmp_arr(FILE *                 fp,
                     stats_result_t const * statsA,
                     stats_result_t const * statsB,
                     uint32_t               to_print,
                     uint32_t const *       percentiles,
                     uint32_t               npercentiles) {
    enum { DEFAULT_BUFLEN = 2048 };
    uint64_t nbytes;
    char     buf[DEFAULT_BUFLEN];
    if (fp == NULL) {
        fp = stdout;
    }
    nbytes = stats_prints_cmp_arr(buf, DEFAULT_BUFLEN, statsA, statsB, to_print,
                                  percentiles, npercentiles);
    die_assert(nbytes < DEFAULT_BUFLEN, "Need larger buf to print stats!\n");
    return safe_fwrite(buf, sizeof(char), nbytes, fp);
}

uint64_t
stats_printf_arr(FILE *                 fp,
                 stats_result_t const * stats,
                 uint32_t               to_print,
                 uint32_t const *       percentiles,
                 uint32_t               npercentiles) {
    enum { DEFAULT_BUFLEN = 1024 };
    uint64_t nbytes;
    char     buf[DEFAULT_BUFLEN];
    if (fp == NULL) {
        fp = stdout;
    }
    nbytes = stats_prints_arr(buf, DEFAULT_BUFLEN, stats, to_print, percentiles,
                              npercentiles);
    die_assert(nbytes < DEFAULT_BUFLEN, "Need larger buf to print stats!\n");
    return safe_fwrite(buf, sizeof(char), nbytes, fp);
}

uint64_t
stats_printf_csv_arr(FILE *                 fp,
                     stats_result_t const * stats,
                     uint32_t               to_print,
                     uint32_t const *       percentiles,
                     uint32_t               npercentiles) {
    enum { DEFAULT_BUFLEN = 1024 };
    uint64_t nbytes;
    char     buf[DEFAULT_BUFLEN];
    if (fp == NULL) {
        fp = stdout;
    }
    nbytes = stats_prints_csv_arr(buf, DEFAULT_BUFLEN, stats, to_print,
                                  percentiles, npercentiles);
    die_assert(nbytes < DEFAULT_BUFLEN, "Need larger buf to print stats!\n");
    return safe_fwrite(buf, sizeof(char), nbytes, fp);
}

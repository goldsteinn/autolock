#include <stdint.h>

#include "util/common.h"
#include "util/inline-math.h"
#include "util/macro.h"
#include "util/random.h"

#include "test/test-common.h"

/********************************************************************/
/* Ensure constant evaluation. */
enum {
    I_const_eval_ROUNDUP      = ROUNDUP(1, 2),
    I_const_eval_ROUNDDOWN    = ROUNDDOWN(1, 2),
    I_const_eval_ROUNDUP_P2   = ROUNDUP_P2(1, 2),
    I_const_eval_ROUNDDOWN_P2 = ROUNDDOWN_P2(1, 2),
    I_const_eval_is_p2        = is_p2(1),
    I_const_eval_p2_factor    = p2_factor(1),
    I_const_eval_CMAX         = CMAX(1, 2, 3),
    I_const_eval_CMIN         = CMIN(1, 2, 3),
};


/********************************************************************/
/* Static tests. */
const_assert(p2_factor(24) == 8);
const_assert(p2_factor(1) == 1);
const_assert(p2_factor(8) == 8);
const_assert(p2_factor(2) == 2);

const_assert(is_p2(0));
const_assert(is_p2(1));
const_assert(is_p2(1U << 31));
const_assert(is_p2(1UL << 63));

const_assert(!is_p2(-1));
const_assert(!is_p2(3));

const_assert(CMAX(1, 2) == 2);
const_assert(CMAX(3, 4) == 4);
const_assert(CMAX(1, -1) == 1);
const_assert(CMAX(1U, -1U) == -1U);
const_assert(CMAX(1UL, -1UL) == -1UL);

const_assert(CMIN(1, 2) == 1);
const_assert(CMIN(3, 4) == 3);
const_assert(CMIN(1, -1) == -1);
const_assert(CMIN(1U, -1U) == 1);
const_assert(CMIN(1UL, -1UL) == 1);

const_assert(CMAX(1, 2, 3) == 3);
const_assert(CMAX(1, 2, 3, 4) == 4);
const_assert(CMAX(1, 2, 3, 4, -1) == 4);
const_assert(CMAX(1, 2, 3, 4, -1U) == -1U);

const_assert(CMIN(1, 2, 3) == 1);
const_assert(CMIN(1, 2, 3, 4) == 1);
const_assert(CMIN(1, 2, 3, 4, -1) == -1);
const_assert(CMIN(1, 2, 3, 4, -1U) == 1);


#define testeq(impl, ref, val)                                                 \
    res   = impl(val);                                                         \
    expec = ref(val);                                                          \
    test_assert(res == expec && is_p2(res) && p2_factor(res) == res,           \
                "%-20s(%lu): %lu != %lu\n", V_TO_STR(ref),                     \
                CAST(uint64_t, (val)), res, expec);


#define testall(v)                                                             \
    testeq(next_p2, simple_next_p2_32, CAST(uint32_t, v));                     \
    testeq(prev_p2, simple_prev_p2_32, CAST(uint32_t, v));                     \
    testeq(next_p2, simple_next_p2_64, CAST(uint64_t, v));                     \
    testeq(prev_p2, simple_prev_p2_64, CAST(uint64_t, v));                     \
    testeq(next_p2, simple_next_p2_64, CAST(int64_t, v));                      \
    testeq(prev_p2, simple_prev_p2_64, CAST(int64_t, v));


static uint64_t
simple_next_p2_64(uint64_t v) {
    v--;
    /* NOLINTBEGIN(*-avoid-magic-numbers) */
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v = v | (v >> 32);
    /* NOLINTEND(*-avoid-magic-numbers) */
    v++;
    return v;
}

static uint64_t
simple_prev_p2_64(uint64_t x) {
    /* NOLINTBEGIN(*-avoid-magic-numbers) */
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    x = x | (x >> 32);
    /* NOLINTEND(*-avoid-magic-numbers) */
    return x - (x >> 1);
}

static uint64_t
simple_next_p2_32(uint64_t v) {
    return CAST(uint32_t, simple_next_p2_64(v));
}

static uint64_t
simple_prev_p2_32(uint64_t v) {
    return CAST(uint32_t, simple_prev_p2_64(v));
}


int32_t
test_log2() {
    uint64_t j, expec;
    for (expec = 0, j = 1; j; ++expec, j <<= 1) {
        test_assert(expec == log2_ru(j), "%lu (%lu != %lu)\n", j, expec,
                    log2_ru(j));
        test_assert(expec == log2_rd(j));
    }

    for (expec = 2, j = 4; j; ++expec, j <<= 1) {
        test_assert(expec == log2_ru(j - 1));
        test_assert(expec - 1 == log2_rd(j - 1));

        test_assert(expec + 1 == log2_ru(j + 1));
        test_assert(expec == log2_rd(j + 1));
    }

    return 0;
}

int32_t
test_max_min() {
    uint64_t ui64;
    uint32_t ui32;
    for (ui64 = 1; ui64; ui64 <<= 1) {
        test_assert(MAX(ui64, ui64 - 1) == ui64);
        test_assert(MAX(ui64, ui64 + 1) == ui64 + 1);
        test_assert(MAX(ui64, ui64 - 1, ui64 + 1) == ui64 + 1);

        test_assert(MIN(ui64, ui64 - 1) == ui64 - 1);
        test_assert(MIN(ui64, ui64 + 1) == ui64);
        test_assert(MIN(ui64, ui64 - 1, ui64 + 1) == ui64 - 1);

        test_assert(CMAX(ui64, ui64 - 1) == ui64);
        test_assert(CMAX(ui64, ui64 + 1) == ui64 + 1);
        test_assert(CMAX(ui64, ui64 - 1, ui64 + 1) == ui64 + 1);

        test_assert(CMIN(ui64, ui64 - 1) == ui64 - 1);
        test_assert(CMIN(ui64, ui64 + 1) == ui64);
        test_assert(CMIN(ui64, ui64 - 1, ui64 + 1) == ui64 - 1);
    }
    for (ui32 = 1; ui32; ui32 <<= 1) {
        test_assert(MAX(ui32, ui32 - 1) == ui32);
        test_assert(MAX(ui32, ui32 + 1) == ui32 + 1);
        test_assert(MAX(ui32, ui32 - 1, ui32 + 1) == ui32 + 1);

        test_assert(MIN(ui32, ui32 - 1) == ui32 - 1);
        test_assert(MIN(ui32, ui32 + 1) == ui32);
        test_assert(MIN(ui32, ui32 - 1, ui32 + 1) == ui32 - 1);

        test_assert(CMAX(ui32, ui32 - 1) == ui32);
        test_assert(CMAX(ui32, ui32 + 1) == ui32 + 1);
        test_assert(CMAX(ui32, ui32 - 1, ui32 + 1) == ui32 + 1);

        test_assert(CMIN(ui32, ui32 - 1) == ui32 - 1);
        test_assert(CMIN(ui32, ui32 + 1) == ui32);
        test_assert(CMIN(ui32, ui32 - 1, ui32 + 1) == ui32 - 1);
    }

    return 0;
}

int32_t
test_p2() {
    uint64_t res, expec, i, j;

    enum { SMALL_BOUND = 128, NUM_LARGE_TESTS = 256 };

    for (i = 0; i < UINT32_MAX;) {
        for (j = 1; j; j <<= 1) {
            testall(i);
            testall(UINT32_MAX - i);
            testall(CAST(uint32_t, ~0) - i);
            testall(~CAST(uint64_t, 0) - i);
            testall(CAST(uint32_t, ~0) + i);
            testall(~CAST(uint64_t, 0) + i);

            testall(i + j);
            testall(UINT32_MAX - i + j);
            testall(CAST(uint32_t, ~0) - i + j);
            testall(~CAST(uint64_t, 0) - i + j);
            testall(CAST(uint32_t, ~0) + i + j);
            testall(~CAST(uint64_t, 0) + i + j);

            testall(i - j);
            testall(UINT32_MAX - i - j);
            testall(CAST(uint32_t, ~0) - i - j);
            testall(~CAST(uint64_t, 0) - i - j);
            testall(CAST(uint32_t, ~0) + i - j);
            testall(~CAST(uint64_t, 0) + i - j);

            testall(j - i);
        }
        if (i < SMALL_BOUND) {
            ++i;
        }
        else {
            i += true_rand32() / NUM_LARGE_TESTS;
        }
    }
    return 0;
}

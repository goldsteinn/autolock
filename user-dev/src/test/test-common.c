#include <math.h>

#include "test/test-common.h"

#include "util/memory-util.h"
#include "util/random.h"


uint8_t *
make_buf(uint64_t sz) {
    uint8_t * buf =
        CAST(uint8_t *, safe_mmap(NULL, 2 * PAGE_SIZE + sz, PROT_NONE,
                                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    safe_mprotect(buf + PAGE_SIZE, sz, PROT_READ | PROT_WRITE);
    return buf + PAGE_SIZE;
}


void
free_buf(uint8_t * buf, uint64_t sz) {
    safe_munmap(buf - PAGE_SIZE, sz + 2 * PAGE_SIZE);
}


void
randomize_buf(uint8_t * buf, uint64_t sz) {
    uint64_t i;
    uint64_t aligned_sz = sz & (-sizeof(uint64_t));
    for (i = 0; i < aligned_sz; i += sizeof(uint64_t)) {
        uint64_t v = rand64();
        __builtin_memcpy(buf + i, &v, sizeof(uint64_t));
    }
    for (; i < sz; ++i) {
        uint8_t v = CAST(uint8_t, rand32());
        buf[i]    = v;
    }
}

void
seq_buf(uint8_t * buf, uint64_t sz) {
    for (uint64_t i = 0; i < sz; ++i) {
        buf[i] = CAST(uint8_t, i);
    }
}

int32_t
is_seq(uint8_t const * buf, uint64_t sz) {
    for (uint64_t i = 0; i < sz; ++i) {
        uint8_t bufi = buf[i];
        if (bufi != CAST(uint8_t, i)) {
            return 0;
        }
    }
    return 1;
}

static __thread int32_t max_ulp = 0;
void
set_max_ulp(int32_t value) {
    max_ulp = value;
}


#define flt_compare(T) CAT(I_float_type_compare_ulp_, T)
#define make_flt_comparer(T)                                                   \
    static int32_t flt_compare(T)(T a, T b) {                                  \
        typedef FLOAT_TO_INT_T(T) fint_t;                                      \
        fint_t a_int, b_int;                                                   \
        if ((a < 0) != (b < 0)) {                                              \
            return 0;                                                          \
        }                                                                      \
        if (isnan(a) && isnan(b)) {                                            \
            return 1;                                                          \
        }                                                                      \
        if (isinf(a) && isinf(b)) {                                            \
            return 1;                                                          \
        }                                                                      \
        if (max_ulp == 0) {                                                    \
            return a == b;                                                     \
        }                                                                      \
        if (max_ulp < 0) {                                                     \
            a -= b;                                                            \
            if (a < 0) {                                                       \
                a = -a;                                                        \
            }                                                                  \
            return a < CAST(T, .00001);                                        \
        }                                                                      \
        /* Do ulp check for final equality check. */                           \
        const_assert(sizeof(a_int) == sizeof(a));                              \
        const_assert(IS_SIGNED(a_int));                                        \
        __builtin_memcpy(&a_int, &a, sizeof(a));                               \
        __builtin_memcpy(&b_int, &b, sizeof(b));                               \
                                                                               \
        if (a_int < 0) {                                                       \
            a_int = TYPEOF_MIN(a_int) - a_int;                                 \
        }                                                                      \
        if (b_int < 0) {                                                       \
            b_int = TYPEOF_MIN(b_int) - b_int;                                 \
        }                                                                      \
        a_int -= b_int;                                                        \
        if (a_int < 0) {                                                       \
            a_int = -a_int;                                                    \
        }                                                                      \
        if (a_int > TYPEOF_MAX(max_ulp) || a_int < TYPEOF_MIN(max_ulp)) {      \
            return 0;                                                          \
        }                                                                      \
        if (CAST(get_type(max_ulp), a_int) > max_ulp) {                        \
            return 0;                                                          \
        }                                                                      \
        return 1;                                                              \
    }

make_flt_comparer(float);
make_flt_comparer(double);

int32_t
I_flt_eq(float a, float b) {
    return flt_compare(float)(a, b);
}

int32_t
I_dbl_eq(double a, double b) {
    return flt_compare(double)(a, b);
}

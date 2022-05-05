#include "lib/algo/sort.h"

#include "util/macro.h"
#include "util/random.h"

#include "test/test-common.h"

/* Mostly exists to test that all the macros work. */
#define sort_test_name(T) CAT(test_sort_, T)
#define run_test(T)       test_assert(sort_test_name(T)() == 0)
#define make_sort_test(T)                                                      \
    static int32_t sort_test_name(T)() {                                       \
        enum { MAX_ITEMS = 1000, NTESTS = 100 };                               \
        T        arr[MAX_ITEMS];                                               \
        uint32_t i, j, k;                                                      \
        for (i = 0; i < MAX_ITEMS; ++i) {                                      \
            arr[i] = i;                                                        \
        }                                                                      \
                                                                               \
        for (i = 0; i < MAX_ITEMS; i = (i ? (((i * 3) / 2) + 1) : 1)) {        \
            if (i > CAST(uint32_t, TYPE_MAX(T))) {                             \
                i = CAST(uint32_t, TYPE_MAX(T));                               \
            }                                                                  \
            for (j = 0; j < NTESTS; ++j) {                                     \
                for (k = 0; k < i; ++k) {                                      \
                    uint32_t idx0 = rand32() % i;                              \
                    uint32_t idx1 = rand32() % i;                              \
                    T        tmp0 = arr[idx0];                                 \
                    T        tmp1 = arr[idx1];                                 \
                                                                               \
                    arr[idx0] = tmp1;                                          \
                    arr[idx1] = tmp0;                                          \
                }                                                              \
                generic_sort(arr, i);                                          \
                for (k = 0; k < i; ++k) {                                      \
                    test_assert(arr[k] == CAST(T, k),                          \
                                "Failure(%s, %u, %u): %lu != %lu\n",           \
                                V_TO_STR(T), k, i, CAST(uint64_t, arr[k]),     \
                                CAST(uint64_t, k));                            \
                }                                                              \
            }                                                                  \
            if (i == CAST(uint32_t, TYPE_MAX(T))) {                            \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

APPLY(make_sort_test, ;, ALL_TYPE_NAMES);

int32_t
test_sort() {
    APPLY(run_test, ;, ALL_TYPE_NAMES);
    return 0;
}

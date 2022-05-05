#include "test/test-common.h"

#include "util/sched-util.h"
#include "util/vdso-util.h"


int32_t
test_vdso() {
    uint32_t        ret;
    struct timespec ts;
    struct timeval  tv;
    struct timezone tz;
    uint32_t        cpu, cpu2, node;
    time_t          tloc;
    uint8_t cmp_bytes[MAX(sizeof(ts), sizeof(tv), sizeof(tz), sizeof(cpu),
                          sizeof(node), sizeof(tloc))];

    ret = vdso_init();
    if (is_vdso_init_error(ret) || ret == 0) {
        warn(
            "Skipping VDSO tests due to inability to initailize. Probably due to valgrind or unsupported arch.\n");
        return 0;
    }

    test_assert(safe_vdso_init() == 0);


    __builtin_memset(cmp_bytes, 0, sizeof(cmp_bytes));
    __builtin_memset(&ts, 0, sizeof(ts));
    __builtin_memset(&tv, 0, sizeof(tv));
    __builtin_memset(&tz, 0, sizeof(tz));
    __builtin_memset(&cpu, 0, sizeof(cpu));
    __builtin_memset(&node, 0, sizeof(node));
    __builtin_memset(&tloc, 0, sizeof(tloc));

    vdso_clock_gettime(CLOCK_MONOTONIC, &ts);
    vdso_gettimeofday(&tv, &tz);
    vdso_getcpu(&cpu, &node);
    cpu2 = get_cpu();
    vdso_time(&tloc);


    test_assert(memcmp(cmp_bytes, &ts, sizeof(ts)));
    test_assert(memcmp(cmp_bytes, &tv, sizeof(tv)));

    test_assert(cpu == cpu2);
    test_assert(memcmp(cmp_bytes, &tloc, sizeof(tloc)));


    vdso_clock_gettime(CLOCK_MONOTONIC, &ts);
    vdso_gettimeofday(&tv, NULL);
    vdso_getcpu(&cpu, NULL);
    cpu2 = get_cpu();
    vdso_time(&tloc);

    test_assert(memcmp(cmp_bytes, &ts, sizeof(ts)));
    test_assert(memcmp(cmp_bytes, &tv, sizeof(tv)));

    test_assert(cpu == cpu2);
    test_assert(memcmp(cmp_bytes, &tloc, sizeof(tloc)));

    return 0;
}

#include "util/random.h"
#include "util/common.h"

#include "test/test-common.h"


int32_t
test_random() {
    volatile uint64_t sink;
    uint64_t          seed, lseed;

    sink = rand32();
    test_assert(sink != rand32(), "Seed not reset!");

    sink = rand64();
    test_assert(sink != rand64());

    enum { SEED_MAX = (1UL << 63), SEED_INCR = 0x0123456789abcdef };
    for (seed = 0; seed < SEED_MAX; seed += SEED_INCR) {
        seed_rand(seed);
        lseed = seed;
        sink  = rand32();

        test_assert(get_rand_seed() != seed);
        test_assert(sink == rand32s(&lseed));
        test_assert(get_rand_seed() == lseed);

        seed_rand(seed);
        test_assert(sink == rand32());
        test_assert(sink == rand32c(seed));

        seed_rand(seed);
        lseed = seed;
        sink  = rand64();

        test_assert(get_rand_seed() != seed);
        test_assert(sink == rand64s(&lseed));
        test_assert(get_rand_seed() == lseed);

        seed_rand(seed);
        test_assert(sink == rand64());
        test_assert(sink == rand64c(seed));
    }

    return 0;
}

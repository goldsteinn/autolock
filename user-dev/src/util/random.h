#ifndef _SRC__UTIL__RANDOM_H_
#define _SRC__UTIL__RANDOM_H_

#include <stdint.h>

#include "util/common.h"


extern __thread uint64_t I_rseed_;

uint64_t true_rand64();

uint64_t true_rand32();


static CONST_FUNC uint64_t
I_rand64c(__uint128_t t) {
    return CAST(uint64_t, t >> 64) ^ CAST(uint64_t, t);
}
static CONST_FUNC uint64_t
rand64c(uint64_t _seed) {
    return I_rand64c(CAST(__uint128_t, _seed) *
                     (_seed ^ CAST(uint64_t, 0xe7037ed1a0b428db)));
}

NONNULL(1) static uint64_t rand64s(uint64_t * seed) {
    uint64_t _seed;
    _seed = *seed;
    *seed = _seed + CAST(uint64_t, 0xa0761d6478bd642f);
    return rand64c(_seed);
}

static uint64_t
rand64() {
    return rand64s(&I_rseed_);
}

/* Haven't found any rand32 library that is particularly optimized so just use
 * the rand64 function for now. */
static CONST_FUNC uint32_t
rand32c(uint64_t _seed) {
    return CAST(uint32_t, rand64c(_seed));
}

NONNULL(1) static uint32_t rand32s(uint64_t * seed) {
    return CAST(uint32_t, rand64s(seed));
}

static uint32_t
rand32() {
    return CAST(uint32_t, rand64());
}

static void
seed_rand(uint64_t _seed) {
    I_rseed_ = _seed;
}

static uint64_t
get_rand_seed() {
    return I_rseed_;
}

#endif

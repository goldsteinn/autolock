#include <stdint.h>
#include <sys/random.h>

#include "util/error-util.h"

__thread uint64_t I_rseed_;

uint64_t
true_rand64() {
    uint64_t _rand_out;
    err_assert(getrandom(&_rand_out, sizeof(uint64_t), 0) == sizeof(uint64_t));
    return _rand_out;
}

uint32_t
true_rand32() {
    uint32_t _rand_out;
    err_assert(getrandom(&_rand_out, sizeof(uint32_t), 0) == sizeof(uint32_t));
    return _rand_out;
}

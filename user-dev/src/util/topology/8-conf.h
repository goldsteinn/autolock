#ifndef _TOPOLOGY_CONF_H_
#define _TOPOLOGY_CONF_H_

/* Generated topology helpers for 8-core lenovo laptop (should be good
 * for Seth & Noah). */

#include <sched.h>
#include <stdint.h>

enum { NUM_NUMA_NODES = 1 };


static uint32_t __attribute__((const))
I_cpu_to_phys_core(uint32_t cpu) {
    if (cpu >= 8) {
        __builtin_unreachable();
    }
    uint32_t var0 = cpu & 3;
    return var0;
}

static uint32_t __attribute__((const)) I_cpu_to_sibling(uint32_t cpu) {
    if (cpu >= 8) {
        __builtin_unreachable();
    }
    uint32_t var0 = cpu ^ 4;
    return var0;
}

static uint32_t __attribute__((const)) I_cpu_to_socket(uint32_t cpu) {
    if (cpu >= 8) {
        __builtin_unreachable();
    }

    uint32_t var0 = 0;
    return var0;
}


static cpu_set_t const * __attribute__((pure))
I_cpu_to_smt_cpu_map(uint32_t cpu) {
    if (cpu >= 8) {
        __builtin_unreachable();
    }
    static const cpu_set_t cpu_to_smt_cpu_map_lut[] = {
        { 0x11 }, { 0x22 }, { 0x44 }, { 0x88 },
        { 0x11 }, { 0x22 }, { 0x44 }, { 0x88 }
    };
    return &(cpu_to_smt_cpu_map_lut[cpu]);
}
static cpu_set_t const * __attribute__((pure))
I_cpu_to_socket_cpu_map(uint32_t cpu) {
    if (cpu >= 8) {
        __builtin_unreachable();
    }
    static const cpu_set_t cpu_to_socket_cpu_map_lut[] = {
        { 0xff }, { 0xff }, { 0xff }, { 0xff },
        { 0xff }, { 0xff }, { 0xff }, { 0xff }
    };
    return &(cpu_to_socket_cpu_map_lut[cpu]);
}
static cpu_set_t const * __attribute__((pure))
I_socket_to_cpu_map(uint32_t socket) {
    if (socket >= 1) {
        __builtin_unreachable();
    }
    static const cpu_set_t socket_to_cpu_map_lut[] = { { 0xff } };
    return &(socket_to_cpu_map_lut[socket]);
}

#endif

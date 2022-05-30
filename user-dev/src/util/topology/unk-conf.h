#ifndef _TOPOLOGY_CONF_H_
#define _TOPOLOGY_CONF_H_

/* Generated topology helpers for 8-core lenovo laptop (should be good
 * for Seth & Noah). */

#include <sched.h>
#include <stdint.h>

enum { NUM_NUMA_NODES = 1 };

static uint32_t __attribute__((noline, noclone, error("Unsupported")))
I_cpu_to_phys_core(uint32_t cpu) {
    return 0;
}

static uint32_t __attribute__((noline, noclone, error("Unsupported")))
I_cpu_to_sibling(uint32_t cpu) {
    return 0;
}

static uint32_t __attribute__((noline, noclone, error("Unsupported")))
I_cpu_to_socket(uint32_t cpu) {
    return 0;
}


static cpu_set_t const * __attribute__((noline,
                                        noclone,
                                        error("Unsupported")))
I_cpu_to_smt_cpu_map(uint32_t cpu) {
    return NULL;
}
static cpu_set_t const * __attribute__((noline,
                                        noclone,
                                        error("Unsupported")))
I_cpu_to_socket_cpu_map(uint32_t cpu) {
    return NULL;
}
static cpu_set_t const * __attribute__((noline,
                                        noclone,
                                        error("Unsupported")))
I_socket_to_cpu_map(uint32_t socket) {
    return NULL;
}

#endif

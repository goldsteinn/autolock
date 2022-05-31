#ifndef _SRC__UTIL__CPU_INFO_H_
#define _SRC__UTIL__CPU_INFO_H_

#include <sched.h>
#include <sys/sysinfo.h>

#include "util/attrs.h"
#include "util/error-util.h"
#include "util/types.h"

#include "arch/ll-getcpu.h"

#ifndef I_SYS_NUM_CPUS
#error "Must have number of CPUs at compile time"
#endif


#if I_SYS_NUM_CPUS == 8
#include "topology/8-conf.h"
#elif I_SYS_NUM_CPUS == 96
#include "topology/96-conf.h"
#else
#error "topology/unk-conf.h"
#endif


static CONST_FUNC uint32_t
logical_id_to_socket(uint32_t logical_id) {
    return I_cpu_to_socket(logical_id);
}

static uint32_t
get_current_socket() {
    return logical_id_to_socket(ll_get_cpu());
}

enum {
    /* For any CPU we are targetting this is true. */
    L1_CACHE_LINE_SIZE = 64
};

#ifndef I_SYS_NUM_CPUS
#define I_SYS_NUM_CPUS 0
#endif


/* Should probably replace with cpuid sometime. */
#if I_SYS_NUM_CPUS == 0

extern uint32_t I_sys_num_cpus;
static uint32_t
get_num_cpus() {
    uint32_t sys_num_cpus = I_sys_num_cpus;
    if (UNLIKELY(sys_num_cpus == 0)) {
        sys_num_cpus   = get_nprocs();
        I_sys_num_cpus = sys_num_cpus;
    }
    return sys_num_cpus;
}
#else
static CONST_FUNC uint32_t
get_num_cpus() {
    return I_SYS_NUM_CPUS;
}
#endif

#if 0 && I_SYS_NUM_CPUS && I_SYS_NUM_CPUS <= 64
#define I_CPU_SET_OPTIMIZED 1
typedef uint64_t cpuset_t;
#else
typedef cpu_set_t cpuset_t;
#endif


enum {
    I_SYS_CORE_MAP_SIZE =
        I_SYS_NUM_CPUS ? I_SYS_NUM_CPUS : sizeof_bits(cpuset_t)
};

typedef INT_HAS_CAPACITY_FOR_T(I_SYS_CORE_MAP_SIZE) cpu_id_t;


//extern cpu_id_t I_sys_logical_to_physical_map[I_SYS_CORE_MAP_SIZE];
#if 0
uint32_t I_slow_get_phys_cpu_id(uint32_t logical_cpu_id);
void     I_slow_get_logical_cpu_ids(uint32_t   physical_cpu_id,
                                    cpuset_t * out);
#endif

static void
get_logical_cpu_ids(uint32_t physical_cpu_id, cpuset_t * out) {
    die_assert(I_SYS_CORE_MAP_SIZE > physical_cpu_id);
#if 0
    I_slow_get_logical_cpu_ids(physical_cpu_id, out);
#else
    __builtin_memcpy(out, I_cpu_to_smt_cpu_map(physical_cpu_id), sizeof(*out));
#endif
}

static uint32_t
get_phys_cpu_id(uint32_t logical_cpu_id) {
#if 0
    uint32_t v;
    die_assert(I_SYS_CORE_MAP_SIZE > logical_cpu_id);

    v = I_sys_logical_to_physical_map[logical_cpu_id];
    if (UNLIKELY(v == 0)) {
        return I_slow_get_phys_cpu_id(logical_cpu_id);
    }

    return (v - 1);
#else
    die_assert(I_SYS_CORE_MAP_SIZE > logical_cpu_id);
    return I_cpu_to_phys_core(logical_cpu_id);
#endif
}


#endif

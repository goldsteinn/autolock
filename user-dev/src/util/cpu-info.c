#include "util/cpu-info.h"
#include "util/file-util.h"

#include "thread/cpuset.h"

#if I_SYS_NUM_CPUS == 0
uint32_t I_sys_num_cores = 0;
#endif

cpu_id_t I_sys_logical_to_physical_map[I_SYS_CORE_MAP_SIZE] = { 0 };

enum { I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE = 64 };

static uint64_t
I_safe_read_sys_file(char const * fmt,
                     uint32_t     cpu_id,
                     uint32_t     base,
                     uint8_t *    bin_buf) {

    char      path_and_buf[I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE];
    char *    end_ptr, *cur_ptr;
    int       fd;
    uint8_t * bin_buf_start = bin_buf;
    uint64_t  sys_info_as_int, nread;


    die_assert(snprintf(path_and_buf, I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE,
                        fmt,
                        cpu_id) < I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE);

    fd = ensure_open(path_and_buf, O_RDONLY);
    __builtin_memset(path_and_buf, 0, I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE);

    nread =
        ensure_read(fd, path_and_buf, I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE);
    die_assert(nread < I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE, "[%lu]\n",
               nread);

    cur_ptr = path_and_buf;
    for (;;) {
        sys_info_as_int = strtoull(cur_ptr, &end_ptr, base);
        if (end_ptr == cur_ptr) {
            break;
        }
        __builtin_memcpy(bin_buf, &sys_info_as_int,
                         sizeof(sys_info_as_int));
        bin_buf += sizeof(sys_info_as_int);

        if ((*end_ptr) == '\0' ||
            (CAST(uint64_t, end_ptr - cur_ptr) >= nread)) {
            break;
        }
        cur_ptr = end_ptr;
    }

    die_assert(end_ptr != path_and_buf);
    safe_close(fd);
    return bin_buf - bin_buf_start;
}


void
I_slow_get_logical_cpu_ids(uint32_t physical_cpu_id, cpuset_t * out) {

    /* Considered caching this but on a many core machine that will be a
     * page of memory wasted for a relatively cold function. If this was
     * ever performance sensative I'd just do it at compile time. */
#if 0
    uint8_t logical_id_bytes[I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE];
    die_assert(physical_cpu_id < get_num_cpus());

    __builtin_memset(logical_id_bytes, 0, sizeof(logical_id_bytes));
    die_assert(I_safe_read_sys_file(
                   "/sys/devices/system/cpu/cpu%u/topology/core_cpus",
                   logical_cpu_id, 16,
                   logical_id_bytes) <= sizeof(*out));


    __builtin_memcpy(out, logical_id_bytes, sizeof(*out));
    __builtin_memcpy(I_sys_physical_to_logical_map + physical_cpu_id,
                     logical_id_bytes, sizeof(cpu_id_map_t));
#endif
    uint32_t i;
    uint32_t nprocs = get_num_cpus();
    cset_zero(out);
    die_assert(physical_cpu_id < nprocs);

    for (i = 0; i < nprocs; ++i) {
        if (get_phys_cpu_id(i) == physical_cpu_id) {
            cset_set(i, out);
        }
    }
}


uint32_t
I_slow_get_phys_cpu_id(uint32_t logical_cpu_id) {
    uint64_t phys_id;
    uint8_t  phys_id_bytes[I_CPU_TOPOLOGY_PATH_AND_BUF_SIZE];
    die_assert(logical_cpu_id < get_num_cpus());

    __builtin_memset(phys_id_bytes, 0, sizeof(phys_id_bytes));
    die_assert(I_safe_read_sys_file(
                   "/sys/devices/system/cpu/cpu%u/topology/core_id",
                   logical_cpu_id, 10,
                   phys_id_bytes) <= sizeof(phys_id));


    phys_id = 0;
    __builtin_memcpy(&phys_id, phys_id_bytes, sizeof(phys_id));

    
    die_assert(CAST(uint64_t, TYPE_MAX(cpu_id_t)) > phys_id);
    I_sys_logical_to_physical_map[logical_cpu_id] =
        CAST(cpu_id_t, phys_id + 1);
    return phys_id;
}

#include "autolock-impls/stats/autolock-stats.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#include "util/cpu-info.h"
#include "util/error-util.h"
#include "util/time-util.h"

#include "lib/algo/sort.h"

typedef struct kernel_autolock_stats_task_entry {
    uint32_t pid : 26;
    uint32_t is_pick : 1;
    uint32_t is_curr : 1;
    uint32_t al_exists : 1;
    uint32_t al_armed : 1;
    uint32_t al_has_pinned : 1;
    uint32_t al_dont_sched : 1;
} kernel_autolock_stats_task_entry_t;

typedef struct kernel_autolock_stats_entry {
    struct timespec                    ts;
    uint32_t                           cpu;
    uint32_t                           nentries;
    kernel_autolock_stats_task_entry_t entries[];
} kernel_autolock_stats_entry_t;


static NONNULL(1) uint64_t I_autolock_stats_entry_get_size(
    kernel_autolock_stats_entry_t const * entry) {
    return sizeof(kernel_autolock_stats_entry_t) +
           entry->nentries * sizeof(kernel_autolock_stats_task_entry_t);
}

static uint64_t
I_autolock_get_buf_size() {
    enum { I_AUTOLOCK_BUF_SIZE = 1 << 19 };
    return I_AUTOLOCK_BUF_SIZE * get_num_cpus();
}

static uint64_t
I_autolock_get_max_entries() {
    return I_autolock_get_buf_size() /
           (sizeof(kernel_autolock_stats_entry_t) +
            sizeof(kernel_autolock_stats_task_entry_t));
}

static uint64_t
I_autolock_get_alloc_size() {
    return I_autolock_get_buf_size() +
           I_autolock_get_max_entries() * sizeof(void *);
}


void
autolock_start_stats() {
    die_assert(
        autolock_init_kstats() == 0,
        "Failure to initialize autolock stats. "
        "Did you compiler the kernel with `WITH_AUTOLOCK_STATS` enabled?\n");
}

void
autolock_free_stats(auto_lock_stats_t * stats) {
    if (stats->base_ptr != NULL) {
        safe_sfree(stats->base_ptr, I_autolock_get_alloc_size());
    }

    __builtin_memset(stats, 0, sizeof(*stats));
}


static NONNULL(1, 2) uint64_t I_autolock_parse_stats(
    uint8_t const * restrict base_ptr,
    kernel_autolock_stats_entry_t const ** restrict entries,
    int64_t expec_bytes) {
    uint64_t entry_idx = 0;
    for (; expec_bytes != 0;) {
        kernel_autolock_stats_entry_t const * cur;
        uint64_t                              cur_sz;

        cur    = CAST(kernel_autolock_stats_entry_t const *, base_ptr);
        cur_sz = I_autolock_stats_entry_get_size(cur);

        entries[entry_idx] = cur;
        ++entry_idx;
        base_ptr += cur_sz;

        expec_bytes -= cur_sz;
        die_assert(expec_bytes >= 0);
    }

    return entry_idx;
}


void
autolock_get_stats(auto_lock_stats_t * stats) {

    kernel_autolock_stats_entry_t const ** entries;

    uint8_t * base_ptr =
        CAST(uint8_t *, safe_zalloc(I_autolock_get_alloc_size()));
    int64_t  err_or_nbytes = autolock_collect_kstats(base_ptr);
    uint64_t nentries;

    die_assert(err_or_nbytes >= 0);

    entries = CAST(kernel_autolock_stats_entry_t const **,
                   base_ptr + I_autolock_get_buf_size());

    nentries = I_autolock_parse_stats(base_ptr, entries, err_or_nbytes);

    stats->entries =
        CAST(kernel_autolock_stats_entry_t const **, entries);
    stats->nentries = nentries;
    stats->base_ptr = base_ptr;
}

static int32_t
I_autolock_stats_entry_cmp(void const * a, void const * b) {
    kernel_autolock_stats_entry_t const *a_entry, *b_entry;
    uint64_t                             a_ns, b_ns;

    a_entry = CAST(kernel_autolock_stats_entry_t const *, a);
    b_entry = CAST(kernel_autolock_stats_entry_t const *, b);


    a_ns = to_ns(&(a_entry->ts));
    b_ns = to_ns(&(b_entry->ts));
    return (a_ns < b_ns) ? (-1) : (a_ns > b_ns ? 1 : 0);
}


static NONNULL(1) int32_t I_autolock_stats_print_entry(
    FILE * restrict fp,
    kernel_autolock_stats_entry_t const * restrict entry,
    int32_t carried_state) {
    uint32_t                                   i, nentries;
    kernel_autolock_stats_task_entry_t const * task_entry;
    nentries = entry->nentries;


    if (1 || carried_state) {
    changed_state:
        fprintf(fp, "Sched Slice (%3u) [%16lu, %16lu]\n", entry->cpu,
                entry->ts.tv_sec, entry->ts.tv_nsec);
        for (i = 0; i < nentries; ++i) {
            task_entry = &(entry->entries[i]);
            fprintf(
                fp,
                "\t%4u: "
                "[pid=%8u, picked=%1u, curr=%1u, al=%1u, armed=%1u, pinned=%1u, dont_sched=%1u]\n",
                i, task_entry->pid, task_entry->is_pick,
                task_entry->is_curr, task_entry->al_exists,
                task_entry->al_armed, task_entry->al_has_pinned,
                task_entry->al_dont_sched);
        }
    }
    else {
        for (i = 0; i < nentries; ++i) {
            task_entry = &(entry->entries[i]);
            if (task_entry->al_exists) {
                carried_state = 1;
                goto changed_state;
            }
        }
    }
    return carried_state;
}

void
autolock_stats_out(FILE * restrict fp,
                   auto_lock_stats_t * restrict stats) {
    uint64_t                               i, nentries;
    kernel_autolock_stats_entry_t const ** entries;
    int32_t                                carrier_print_state = 0;
    if (stats->base_ptr == NULL) {
        warn("Stats should have been collected before outputting\n");
        return;
    }

    if (fp == NULL) {
        fp = stdout;
    }

    entries  = stats->entries;
    nentries = stats->nentries;
    qsort(entries, nentries, sizeof(void *),
          &I_autolock_stats_entry_cmp);

    for (i = 0; i < nentries; ++i) {
        carrier_print_state = I_autolock_stats_print_entry(
            fp, entries[i], carrier_print_state);
    }
}

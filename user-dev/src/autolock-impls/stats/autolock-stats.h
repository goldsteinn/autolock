#ifndef _SRC__AUTOLOCK_IMPLS__STATS__AUTOLOCK_STATS_H_
#define _SRC__AUTOLOCK_IMPLS__STATS__AUTOLOCK_STATS_H_

#include "util/common.h"

struct kernel_autolock_stats_entry;
typedef struct auto_lock_stats {
    struct kernel_autolock_stats_entry const ** entries;
    uint64_t                                    nentries;
    uint8_t *                                   base_ptr;
} auto_lock_stats_t;


void autolock_start_stats();
void NONNULL(1) autolock_free_stats(auto_lock_stats_t * stats);
void NONNULL(1) autolock_get_stats(auto_lock_stats_t * stats);
void NONNULL(2) autolock_stats_out(FILE * restrict fp,
                                   auto_lock_stats_t * restrict stats);

#endif

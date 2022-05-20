#ifndef _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CBMOCS_LOCK_BASE_H_
#define _SRC__AUTOLOCK_IMPLS__LOCKS__INTERNAL__CBMOCS_LOCK_BASE_H_

/* NB: This file is meant to be included by another with I_WITH_AUTOLOCK
 * as zero or non-zero. */

#ifndef I_WITH_AUTOLOCK
#error "Never include clh-lock-base.h directly!"
#endif

/*
 * David Dice, Virendra J. Marathe, and Nir Shavit. 2015.
 * Lock Cohorting: A General Technique for Designing NUMA Locks.
 * ACM Trans. Parallel Comput. 1, 2, Article 13 (February 2015).
 *
 * Roughly, the idea of C-BO-MCS is to have one lock per NUMA node, and
 * one global lock.
 * - On lock, the first time the thread grabs its local lock and then
 * the global lock
 * - On unlock, the thread releases the local lock, and if there is no
 * thread waiting for the local lock, it releases the global lock (or
 * for fairness it releases the global lock randomly)
 * - On lock, if the thread that unlocks the local lock doesn't release
 * the global lock, then the locker thread doesn't need to grab the
 * global lock and just grabs the local one.
 * - Here the local lock is a MCS lock and the global lock is a Backoff
 * lock
 */


#include "util/arch.h"
#include "util/attrs.h"
#include "util/cpu-info.h"

extern_C_start();
#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/sys/autolock-kernel-api.h"


static CONST_FUNC uint32_t
cpu_id_to_num_node(uint32_t cpu_id) {
    /* This is patently incorrect for many systems. Will fix later. */
    return (cpu_id / (get_num_cpus() / NUM_NUMA_NODES));
}

static uint32_t
get_current_numa_node() {
    return cpu_id_to_num_node(ll_get_cpu());
}

static uint32_t mcs_lock_base_init(mcs_lock_t * lock) {

}

#endif

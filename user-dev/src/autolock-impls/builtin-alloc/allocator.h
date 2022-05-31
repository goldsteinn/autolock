#ifndef _SRC__AUTOLOCK_IMPLS__BUILTIN_ALLOC__ALLOCATOR_H_
#define _SRC__AUTOLOCK_IMPLS__BUILTIN_ALLOC__ALLOCATOR_H_

/* Simple allocator for queue locks that reuses leftover page allocated
 * with the autolock. By design it expects serialized allocation (i.e
 * only owner thread can alloc, any thread can free). This could easily
 * be turned into a generalized allocator. */

#include "autolock-impls/common/autolock-returns.h"
#include "autolock-impls/sys/autolock-kernel-api.h"

#include "util/cpu-info.h"
#include "util/inline-math.h"
#include "util/types.h"

static uint8_t *
I_allocator_get_region_start() {
    return CAST(uint8_t *, I_kernel_autolock) + L1_CACHE_LINE_SIZE * 2;
}

static safe_uint64_t *
I_allocator_get_cur_mask_ptr(uint8_t * region_start) {
    return CAST(safe_uint64_t *, region_start - sizeof(uint64_t));
}

static uint64_t
I_allocator_ptr_to_bit(uint8_t * ptr) {
    uint32_t ptr_int32 = CAST(uint32_t, CAST(uint64_t, ptr));
    ptr_int32 /= L1_CACHE_LINE_SIZE;
    /* In reality this this is a nop because x86 shifts automatically
     * truncate shifts to lower bits. All modern compilers with
     * optimization on do this for us. */
    ptr_int32 &= (4096 / L1_CACHE_LINE_SIZE) - 1;
    return CAST(uint64_t, 1) << ptr_int32;
}

static safe_uint64_t *
I_allocator_ptr_get_mask_ptr(uint8_t * ptr) {
    uint64_t ptr_int = CAST(uint64_t, ptr);
    ptr_int &= (-4096); /* page size. */
    ptr_int += L1_CACHE_LINE_SIZE * 2 - sizeof(uint64_t);
    return CAST(safe_uint64_t *, ptr_int);
}


static uint32_t
autolock_allocator_init() {
    if (UNLIKELY(autolock_init_kernel_state() == NULL)) {
        return I_FAILURE;
    }
    __atomic_store_n(
        I_allocator_get_cur_mask_ptr(I_allocator_get_region_start()),
        -4UL /* first 2 cache lines already used. */, __ATOMIC_RELAXED);

    return I_SUCCESS;
}

static MALLOC_FUNC uint8_t *
autolock_allocator_alloc() {
    uint8_t *       region_start = I_allocator_get_region_start();
    safe_uint64_t * cur_mask_ptr =
        I_allocator_get_cur_mask_ptr(region_start);

    uint64_t cur_mask = *cur_mask_ptr;

    /* Its okay if lsb get updated by a free. */
    uint64_t lsb = cur_mask & (-cur_mask);

    /* Single allocator so no need for CAS. */
    __atomic_fetch_add(cur_mask_ptr, lsb, __ATOMIC_RELAXED);

    return region_start + ctz(lsb) * L1_CACHE_LINE_SIZE;
}


static MALLOC_FUNC void
autolock_allocator_free(uint8_t * ptr) {
    safe_uint64_t * mask_ptr = I_allocator_ptr_get_mask_ptr(ptr);
    uint64_t ptr_bit = I_autolock_allocator_ptr_to_bit(ptr);
    /* We are exclusive owner of this bit and it must be set so just
     * subtract. */
    __atomic_fetch_sub(cur_mask_ptr, ptr_bit, __ATOMIC_RELAXED);
}


#endif

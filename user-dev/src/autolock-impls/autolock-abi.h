#ifndef _SRC__AUTLOCK_IMPLS__AUTOLOCK_ABI_H_
#define _SRC__AUTLOCK_IMPLS__AUTOLOCK_ABI_H_

/* Kernel ABI. */

#include <stdint.h>

struct kernel_autolock_abi {
    uint32_t * watch_mem; /* VM address of memory to watch. */
    uint32_t   watch_for; /* Value to watch for. */
    uint32_t   watch_neq; /* Watch for memory and value to be equal or
                             not-equal   (0 and 1 respectively). */
};

#endif

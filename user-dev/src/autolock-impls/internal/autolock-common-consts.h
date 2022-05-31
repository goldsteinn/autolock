#ifndef _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_COMMON_CONSTS_H_
#define _SRC__AUTOLOCK_IMPLS__INTERNAL__AUTOLOCK_COMMON_CONSTS_H_

enum { I_SUCCESS = 0, I_FAILURE = 1 };

#ifdef I_USE_FOR_SHARED_LIBRARY_INTERPOSE
/* For compatibility with PTHREAD_MUTEX_INITIALIZER must be I_UNLOCKED =
 * 0, I_LOCKED = 1. This is not ideal (from efficiency point of view)
 * but is necessary for benchmarking. */
enum { I_UNLOCKED = 0, I_LOCKED = 1 };

#else

enum { I_UNLOCKED = 1, I_LOCKED = 0 };

#endif


#endif

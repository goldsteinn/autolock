#ifndef _SRC__INTERPOSE__PTHREAD_MUTEX_API_H_
#define _SRC__INTERPOSE__PTHREAD_MUTEX_API_H_

#include <pthread.h>

#include "util/attrs.h"
#include "util/common.h"
#include "util/macro.h"
#include "util/types.h"

/* WARNING:

   pthread_mutex_t is often initialized with PTHREAD_MUTEX_INITIALIZER.
   This is generally all zeros but can vary by system/version.

 */

    extern_C_start();
static NONNULL(1) int I_pthread_mutex_init(
    pthread_mutex_t *           mutex,
    const pthread_mutexattr_t * mutexattr);
static NONNULL(1) int I_pthread_mutex_destroy(pthread_mutex_t * mutex);
static NONNULL(1) int I_pthread_mutex_trylock(pthread_mutex_t * mutex);
static NONNULL(1) int I_pthread_mutex_lock(pthread_mutex_t * mutex);
static NONNULL(1) int I_pthread_mutex_unlock(pthread_mutex_t * mutex);


static NONNULL(1, 2) int I_pthread_mutex_getprioceiling(
    const pthread_mutex_t * mutex,
    int *                   vptr);
static NONNULL(1, 3) int I_pthread_mutex_setprioceiling(
    pthread_mutex_t * mutex,
    int,
    int * vptr);
static NONNULL(1) int I_pthread_mutexattr_destroy(
    pthread_mutexattr_t * mutexattr);
static NONNULL(1, 2) int I_pthread_mutexattr_getprioceiling(
    const pthread_mutexattr_t * mutexattr,
    int *                       vptr);
static NONNULL(1, 2) int I_pthread_mutexattr_getprotocol(
    const pthread_mutexattr_t * mutexattr,
    int *                       vptr);
static NONNULL(1, 2) int I_pthread_mutexattr_getpshared(
    const pthread_mutexattr_t * mutexattr,
    int *                       vptr);
static NONNULL(1, 2) int I_pthread_mutexattr_gettype(
    const pthread_mutexattr_t * mutexattr,
    int *                       vptr);
static NONNULL(1) int I_pthread_mutexattr_init(
    pthread_mutexattr_t * mutexattr);
static NONNULL(1) int I_pthread_mutexattr_setprioceiling(
    pthread_mutexattr_t * mutexattr,
    int                   v);
static NONNULL(1) int I_pthread_mutexattr_setprotocol(
    pthread_mutexattr_t * mutexattr,
    int                   v);
static NONNULL(1) int I_pthread_mutexattr_setpshared(
    pthread_mutexattr_t * mutexattr,
    int                   v);
static NONNULL(1) int I_pthread_mutexattr_settype(
    pthread_mutexattr_t * mutexattr,
    int);
extern_C_end();

#endif

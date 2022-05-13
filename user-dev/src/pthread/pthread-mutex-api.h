#ifndef I_LOCK_T
#error "No lock type"
#endif

#ifndef I_LOCK_INIT
#error "No init function"
#endif

#ifndef I_LOCK_DESTROY
#error "No destroy function"
#endif

#ifndef I_LOCK_LOCK
#error "No lock function"
#endif

#ifndef I_LOCK_TRYLOCK
#error "No trylock function"
#endif

#ifndef I_LOCK_UNLOCK
#error "No trylock function"
#endif


static NONNULL(1) int I_pthread_mutex_destroy(pthread_mutex_t *);
static NONNULL(1, 2) int I_pthread_mutex_getprioceiling(
    const pthread_mutex_t *,
    int *);
static NONNULL(1,
               2) int I_pthread_mutex_init(pthread_mutex_t *,
                                           const pthread_mutexattr_t *);
static NONNULL(1) int I_pthread_mutex_lock(pthread_mutex_t *);
static NONNULL(1,
               3) int I_pthread_mutex_setprioceiling(pthread_mutex_t *,
                                                     int,
                                                     int *);
static NONNULL(1) int I_pthread_mutex_trylock(pthread_mutex_t *);
static NONNULL(1) int I_pthread_mutex_unlock(pthread_mutex_t *);
static NONNULL(1) int I_pthread_mutexattr_destroy(
    pthread_mutexattr_t *);
static NONNULL(1, 2) int I_pthread_mutexattr_getprioceiling(
    const pthread_mutexattr_t *,
    int *);
static NONNULL(1, 2) int I_pthread_mutexattr_getprotocol(
    const pthread_mutexattr_t *,
    int *);
static NONNULL(1, 2) int I_pthread_mutexattr_getpshared(
    const pthread_mutexattr_t *,
    int *);
static NONNULL(1, 2) int I_pthread_mutexattr_gettype(
    const pthread_mutexattr_t *,
    int *);
static NONNULL(1) int I_pthread_mutexattr_init(pthread_mutexattr_t *);
static NONNULL(1) int I_pthread_mutexattr_setprioceiling(
    pthread_mutexattr_t *,
    int);
static NONNULL(
    1) int I_pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
static NONNULL(
    1) int I_pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
static NONNULL(1) int I_pthread_mutexattr_settype(pthread_mutexattr_t *,
                                                  int);


const_assert(sizeof(I_LOCK_T) <= sizeof(pthread_mutex_t));

typedef union I_pthread_mutex {
    pthread_mutex_t glibc_mutex;
    I_LOCK_T        custom_mutex;
} I_pthread_mutex_t __attribute__((may_alias));

static int
I_pthread_mutex_init(pthread_mutex_t *           mutex,
                     const pthread_mutexattr_t * mutexattr) {
    I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;

    /* For now we entirely ignore the attr. */
    (void)mutexattr;

    return I_LOCK_INIT(&(mutex_u->custom_mutex));
}

static int
I_pthread_mutex_destroy(pthread_mutex_t * mutex) {
    I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;
    return I_LOCK_DESTROY(&(mutex_u->custom_mutex));
}

static int
I_pthread_mutex_lock(pthread_mutex_t * mutex) {
    I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;
    return I_LOCK_LOCK(&(mutex_u->custom_mutex));
}

static int
I_pthread_mutex_trylock(pthread_mutex_t * mutex) {
    I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;
    return I_LOCK_TRYLOCK(&(mutex_u->custom_mutex));
}

static int
I_pthread_mutex_unlock(pthread_mutex_t * mutex) {
    I_pthread_mutex_t * mutex_u = (I_pthread_mutex_t *)mutex;
    return I_LOCK_UNLOCK(&(mutex_u->custom_mutex));
}


int
pthread_mutex_init(pthread_mutex_t *           mutex,
                   const pthread_mutexattr_t * mutexattr) {
    return I_pthread_mutex_init(mutex, mutexattr);
}

int
pthread_mutex_destroy(pthread_mutex_t * mutex) {
    return I_pthread_mutex_destroy(mutex);
}

int
pthread_mutex_lock(pthread_mutex_t * mutex) {
    return I_pthread_mutex_lock(mutex);
}

int
pthread_mutex_trylock(pthread_mutex_t * mutex) {
    return I_pthread_mutex_trylock(mutex);
}

int
pthread_mutex_unlock(pthread_mutex_t * mutex) {
    return I_pthread_mutex_unlock(mutex);
}

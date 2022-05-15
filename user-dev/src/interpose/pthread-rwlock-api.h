#error "Not implemented!"

static NONNULL(1) int I_pthread_rwlock_destroy(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_init(
    pthread_rwlock_t *,
    const pthread_rwlockattr_t *);
static NONNULL(1) int I_pthread_rwlock_rdlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_tryrdlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_trywrlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_unlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_wrlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlockattr_destroy(
    pthread_rwlockattr_t *);
static NONNULL(1, 2) int I_pthread_rwlockattr_getpshared(
    const pthread_rwlockattr_t *,
    int *);
static NONNULL(1) int I_pthread_rwlockattr_init(pthread_rwlockattr_t *);
static NONNULL(
    1) int I_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
static NONNULL(1) int I_pthread_rwlock_destroy(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_init(
    pthread_rwlock_t *,
    const pthread_rwlockattr_t *);
static NONNULL(1) int I_pthread_rwlock_rdlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_tryrdlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_trywrlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_unlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlock_wrlock(pthread_rwlock_t *);
static NONNULL(1) int I_pthread_rwlockattr_destroy(
    pthread_rwlockattr_t *);
static NONNULL(1, 2) int I_pthread_rwlockattr_getpshared(
    const pthread_rwlockattr_t *,
    int *);
static NONNULL(1) int I_pthread_rwlockattr_init(pthread_rwlockattr_t *);
static NONNULL(
    1) int I_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);


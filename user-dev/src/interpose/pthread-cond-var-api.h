#error "Not implemented"

static NONNULL(1) int I_pthread_cond_broadcast(pthread_cond_t *);
static NONNULL(1) int I_pthread_cond_destroy(pthread_cond_t *);
static NONNULL(1) int I_pthread_cond_init(pthread_cond_t *,
                                          const pthread_condattr_t *);
static NONNULL(1) int I_pthread_cond_signal(pthread_cond_t *);
static NONNULL(1,
               2,
               3) int I_pthread_cond_timedwait(pthread_cond_t *,
                                               pthread_mutex_t *,
                                               const struct timespec *);
static NONNULL(1, 2) int I_pthread_cond_wait(pthread_cond_t *,
                                             pthread_mutex_t *);
static NONNULL(1) int I_pthread_condattr_destroy(pthread_condattr_t *);
static NONNULL(1, 2) int I_pthread_condattr_getpshared(
    const pthread_condattr_t *,
    int *);
static NONNULL(1) int I_pthread_condattr_init(pthread_condattr_t *);
static NONNULL(
    1) int I_pthread_condattr_setpshared(pthread_condattr_t *, int);
static NONNULL(1) int I_pthread_cond_broadcast(pthread_cond_t *);
static NONNULL(1) int I_pthread_cond_destroy(pthread_cond_t *);
static NONNULL(1) int I_pthread_cond_init(pthread_cond_t *,
                                          const pthread_condattr_t *);
static NONNULL(1) int I_pthread_cond_signal(pthread_cond_t *);
static NONNULL(1,
               2,
               3) int I_pthread_cond_timedwait(pthread_cond_t *,
                                               pthread_mutex_t *,
                                               const struct timespec *);
static NONNULL(1, 2) int I_pthread_cond_wait(pthread_cond_t *,
                                             pthread_mutex_t *);
static NONNULL(1) int I_pthread_condattr_destroy(pthread_condattr_t *);
static NONNULL(1, 2) int I_pthread_condattr_getpshared(
    const pthread_condattr_t *,
    int *);
static NONNULL(1) int I_pthread_condattr_init(pthread_condattr_t *);
static NONNULL(
    1) int I_pthread_condattr_setpshared(pthread_condattr_t *, int);


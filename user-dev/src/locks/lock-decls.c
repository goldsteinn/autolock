/* Third part. */
#include "absl/synchronization/mutex.h"
#include "folly/MicroLock.h"

/* Internal. */
#include "locks/lock-base.h"
#include "locks/lock-bench.h"

#include "autolock-impls/autolock-export.h"

#include "util/func-decl-generator.h"


/* All lock declarations must be included here and appended below to
 * LOCK_IMPLS. All locks must adhere to the following API:
 *      1. static __typeof__(this) init(void *)
 *      2. void destroy()
 *      3. int try_lock()
 *      4. void lock()
 *      5. void unlock()
 */


/* Macro for generating lock implementing as config of `class lock_ops`
 * (see lock-base.h). */

#define gen_lock(name, max_raw_iter, max_yield_iter, with_backoff,     \
                 with_yield, with_futex)                               \
    class name {                                                       \
        using interface = lock_ops<max_raw_iter,                       \
                                   max_yield_iter,                     \
                                   with_backoff,                       \
                                   with_yield,                         \
                                   with_futex>;                        \
        lock_base_t lock_internal;                                     \
                                                                       \
       public:                                                         \
        static name *                                                  \
        init(void * init_mem) {                                        \
            name * lock = CAST(name *, init_mem);                      \
            interface::init(&(lock->lock_internal));                   \
            return lock;                                               \
        }                                                              \
        int32_t                                                        \
        try_lock() {                                                   \
            return interface::try_lock(&lock_internal);                \
        }                                                              \
        void                                                           \
        lock() {                                                       \
            interface::lock(&lock_internal);                           \
        }                                                              \
        void                                                           \
        unlock() {                                                     \
            interface::unlock(&lock_internal);                         \
        }                                                              \
        void                                                           \
        destroy() {                                                    \
            /* nop. */                                                 \
        }                                                              \
    } __attribute__((may_alias));


/* Common configs worth testing. */
gen_lock(spinlock, 0, 0, 0, 0, 0);
gen_lock(backoff_spinlock, 0, 0, 1, 0, 0);

gen_lock(yield_spinlock, 16, 0, 0, 1, 0);
gen_lock(yield_backoff_spinlock, 16, 0, 1, 1, 0);

gen_lock(futex_spinlock, 16, 4, 0, 0, 1);
gen_lock(futex_backoff_spinlock, 16, 4, 1, 0, 1);

gen_lock(futex_yield_spinlock, 16, 4, 0, 1, 1);
gen_lock(futex_yield_backoff_spinlock, 16, 4, 1, 1, 1);


/* Lock API build on pthread_mutex. */
class pthread_mutex {
    pthread_mutex_t m;

   public:
    static pthread_mutex *
    init(void * init_mem) {
        pthread_mutex * lock = CAST(pthread_mutex *, init_mem);
        die_assert(pthread_mutex_init(&(lock->m), NULL) == 0);
        return lock;
    }

    int32_t
    try_lock() {
        return pthread_mutex_trylock(&m);
    }

    void
    lock() {
        pthread_mutex_lock(&m);
    }

    void
    unlock() {
        pthread_mutex_unlock(&m);
    }

    void
    destroy() {
        pthread_mutex_destroy(&m);
    }
} __attribute__((may_alias));


/* Lock API build on pthread_spinlock. */
class pthread_spinlock {
    pthread_spinlock_t m;

   public:
    static pthread_spinlock *
    init(void * init_mem) {
        pthread_spinlock * lock = CAST(pthread_spinlock *, init_mem);
        die_assert(pthread_spin_init(&(lock->m), 0) == 0);
        return lock;
    }

    int32_t
    try_lock() {
        return pthread_spin_trylock(&m);
    }
    void
    lock() {
        pthread_spin_lock(&m);
    }

    void
    unlock() {
        pthread_spin_unlock(&m);
    }

    void
    destroy() {
        pthread_spin_destroy(&m);
    }
} __attribute__((may_alias));

/* Lock API build on google abseil's mutex. */
class abseil_mutex {
    absl::Mutex m;

   public:
    static abseil_mutex *
    init(void * init_mem) {
        abseil_mutex * lock = new (init_mem) abseil_mutex;
        return lock;
    }

    int32_t
    try_lock() {
        return m.TryLock();
    }
    void
    lock() {
        m.Lock();
    }

    void
    unlock() {
        m.Unlock();
    }

    void
    destroy() {
        m.~Mutex();
    }
} __attribute__((may_alias));

/* Lock API build on facebook folly's mutex. */
class folly_mutex {
    folly::MicroLock m;

   public:
    static folly_mutex *
    init(void * init_mem) {
        folly_mutex * lock = new (init_mem) folly_mutex;
        return lock;
    }

    int32_t
    try_lock() {
        return m.try_lock();
    }
    void
    lock() {
        m.lock();
    }

    void
    unlock() {
        m.unlock();
    }

    void
    destroy() {
        /* nop. */
    }
} __attribute__((may_alias));


/* All implementations (AUTOLOCK_IMPLS defined in
 * autolock-impls/autolock-export.h. */

#define LOCK_IMPLS                                                     \
    pthread_mutex, pthread_spinlock, spinlock, backoff_spinlock,       \
        yield_spinlock, yield_backoff_spinlock, futex_spinlock,        \
        futex_backoff_spinlock, futex_yield_spinlock,                  \
        futex_yield_backoff_spinlock, folly_mutex, abseil_mutex,       \
        AUTOLOCK_IMPLS


#define make_lock_info(class_name)                                     \
    {                                                                  \
        V_TO_STR(class_name), {                                        \
            CAST(void const *, &bench_runner<class_name>)              \
        }                                                              \
    }

/* Declare all locks in LOCK_IMPLS use the above `make_lock_info` macro.
 * This gets us a pointer to each of their templated bench_runner
 * functions. */
custom_make_decls(
    decl_list_t,    /* Type (list with func pointers + name. */
    lock_list,      /* Variable name for the list. */
    make_lock_info, /* Initialization macro. */
    EAT,            /* Just eat args (we don't need fwd declare). */
    LOCK_IMPLS      /* All lock impls. */
);

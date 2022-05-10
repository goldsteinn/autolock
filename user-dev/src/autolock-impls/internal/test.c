static NONNULL(1) void rseq_autolock_lock(rseq_autolock_t * lock) {
    asm volatile goto(
        RSEQ_END_ABORT_DEF()
        : "+m"((uint32_t(*)[1])(&(lock->mem))),
          "=m"((uint32_t * (*)[1])(&(I_kernel_autolock->watch_mem))),
          "=m"((uint32_t(*)[1])(&(I_kernel_autolock->watch_for))),
          "=m"((uint32_t(*)[1])(&(I_kernel_autolock->watch_neq)))
        : (lock_mem) "r"(lock)
        : "eax", "edx", "cc"
        : acquired);
acquired:
}

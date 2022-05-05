#include <dlfcn.h>

#include "util/error-util.h"
#include "util/vdso-util.h"

#include "util/internal/timers.h"


I_vdso_placeholder_f vdso_funcs[] = {
    CAST(I_vdso_placeholder_f, &direct_clock_gettime),
    CAST(I_vdso_placeholder_f, &gettimeofday),
    CAST(I_vdso_placeholder_f, &getcpu), CAST(I_vdso_placeholder_f, &time)
};


static uint32_t
set_vdso_func(I_vdso_placeholder_f fptr, size_t offset) {
    if (fptr != NULL) {
        vdso_funcs[offset] = fptr;
        return 1U << offset;
    }
    return 0;
}

uint32_t
safe_vdso_init() {
#ifdef WITH_VDSO
    uint32_t ret = vdso_init();
    err_assert(!is_vdso_init_error(ret), "%s\n",
               dlerror()); /* NOLINT(concurrency-mt-unsafe) */

    if (ret != get_vdso_expec_mask()) {
        return ret;
    }
#else
    warn("VDSO is disabled!\n");
#endif
    return 0;
}

void
safe_vdso_init_all() {
    uint32_t ret = safe_vdso_init();
    die_assert(ret == 0, "Only partially initialized\n");
}

uint32_t
vdso_init() {
    uint32_t ret = 0;
#ifdef WITH_VDSO
    void * vdso_lib;
    vdso_lib = CAST(void *, dlopen("linux-vdso.so.1",
                                   RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD));
    if (vdso_lib == NULL) {
        return -1U;
    }

    ret |= set_vdso_func(
        CAST(I_vdso_placeholder_f, dlsym(vdso_lib, "__vdso_clock_gettime")),
        vdso_clock_gettime_offset);
    ret |= set_vdso_func(
        CAST(I_vdso_placeholder_f, dlsym(vdso_lib, "__vdso_gettimeofday")),
        vdso_gettimeofday_offset);
    ret |= set_vdso_func(
        CAST(I_vdso_placeholder_f, dlsym(vdso_lib, "__vdso_getcpu")),
        vdso_getcpu_offset);
    ret |= set_vdso_func(
        CAST(I_vdso_placeholder_f, dlsym(vdso_lib, "__vdso_time")),
        vdso_time_offset);

    dlclose(vdso_lib);
#else
    warn("VDSO is disabled!\n");
#endif
    return ret;
}

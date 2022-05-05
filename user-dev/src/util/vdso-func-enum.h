#ifndef _SRC__UTIL__VDSO_FUNC_ENUM_H_
#define _SRC__UTIL__VDSO_FUNC_ENUM_H_

enum {
    vdso_clock_gettime_offset = 0,
    vdso_gettimeofday_offset  = 1,
    vdso_getcpu_offset        = 2,
    vdso_time_offset          = 3
};


#endif

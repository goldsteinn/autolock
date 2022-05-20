#ifndef _SRC__UTIL__ATTRS_H_
#define _SRC__UTIL__ATTRS_H_

#include "util/macro.h"
#include "util/portability.h"

#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE  __attribute__((noinline))


#define MAYBE_UNUSED __attribute__((unused))
#define MALLOC_FUNC  __attribute__((malloc))
#define CONST_FUNC   __attribute__((const)) constexpr
#define PURE_FUNC    __attribute__((pure))

#define BENCH_FUNC __attribute__((noclone, noinline, aligned(4096)))
#define EXIT_FUNC  __attribute__((noclone, noinline, cold, noreturn))

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define FORMATF(...) __attribute__((format(printf, ##__VA_ARGS__)))

#define ATTR_COPY(...) I_attribute_copy(__VA_ARGS__)

#define ATTR_TLS(...) __attribute__((tls_model(__VA_ARGS__)))

#define ATTR_TLS_INIT_EXEC ATTR_TLS("initial-exec")
#endif

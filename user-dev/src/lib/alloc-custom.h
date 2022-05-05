#ifndef _SRC__LIB__ALLOC_CUSTOM_H_
#define _SRC__LIB__ALLOC_CUSTOM_H_

#include <malloc.h>

#define malloc_c  malloc
#define realloc_c realloc
#define srealloc_c(p, so, sn)                                                  \
    ({                                                                         \
        UNUSED(so);                                                            \
        realloc(p, sn);                                                        \
    })
#define calloc_c        calloc
#define aligned_alloc_c aligned_alloc

#define free_c free
#define sfree_c(p, s)                                                          \
    UNUSED(s);                                                                 \
    free(p)

#endif

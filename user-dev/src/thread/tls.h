#ifndef _SRC__THREAD__TLS_H_
#define _SRC__THREAD__TLS_H_

#include "util/types.h"

#include "arch/ll-tls.h"

static uint8_t *
get_tls_start(void) {
    return CAST(uint8_t *, ll_tls_start());
}


#endif

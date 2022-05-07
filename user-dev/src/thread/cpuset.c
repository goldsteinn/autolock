#include "thread/cpuset.h"

char *
cset_to_str(char * buf, uint64_t buflen, cpuset_t const * cset) {
    uint64_t nwritten;
    int32_t  cpu;
    cpuset_t tmp;
    cset_copy(&tmp, cset);
    die_assert(buflen);
    nwritten = snprintf(buf, buflen, "[");

    while (!cset_empty(&tmp)) {
        cpu = cset_first(&tmp);
        die_assert(cpu != -1);
        die_assert(nwritten < buflen);
        nwritten += snprintf(buf + nwritten, buflen - nwritten, "%s%4d",
                             nwritten <= 1 ? "" : ",", cpu);
        cset_clr(cpu, &tmp);
    }
    die_assert(nwritten < buflen);
    nwritten += snprintf(buf + nwritten, buflen, "]");
    die_assert(nwritten < buflen);
    return buf;
}

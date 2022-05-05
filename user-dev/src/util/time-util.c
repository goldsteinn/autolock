#include "util/time-util.h"


void
print_res(char const * desc,
          uint64_t     time,
          uint64_t     trials,
          char const * units) {
    char const *_desc = "", *_units = "";
    if (desc) {
        _desc = desc;
    }
    if (units) {
        _units = units;
    }

    printf("%-24s -> %10.3lf %s\n", _desc,
           CAST(double, time) / CAST(double, trials), _units);
}

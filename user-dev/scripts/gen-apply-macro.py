#! /usr/bin/env python3

import sys

assert len(sys.argv) > 3

macro_base = sys.argv[1]
macro_action = sys.argv[2]
macro_sep = sys.argv[3]
macro_args = ",".join(sys.argv[4:])

xargs = []
actions = []
for i in range(1, 64):
    xarg = "x{}".format(i)
    xargs.append(xarg)
    actions.append(macro_action.replace("ARG", xarg))
    print("#define {}_{}({}, {}) {}".format(macro_base, i, macro_args,
                                            ", ".join(xargs),
                                            (macro_sep + " ").join(actions)))

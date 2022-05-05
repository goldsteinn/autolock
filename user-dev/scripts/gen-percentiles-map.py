#! /usr/bin/env python3

import sys

supported = {0, 1, 5, 10, 25, 50, 75, 90, 95, 99, 100}

vals = []

for i in range(0, 101):
    if i in supported:
        vals.append("I_P{}_IDX".format(i))
    else:
        vals.append("0xff")

print(
    "/* Generated with '%s'. */\nstatic const uint8_t percentile_indexes[] = { %s };"
    % (sys.argv[0], ", ".join(vals)))

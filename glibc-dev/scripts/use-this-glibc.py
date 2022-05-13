import sys
import os

assert len(sys.argv) > 2, "Usage: <path to glibc install> <executable>"

glibc_path = os.path.realpath(sys.argv[1])
exe_path = os.path.realpath(sys.argv[2])

#lib/ld-linux-x86-64.so.2
assert os.path.isdir(glibc_path)
ld_path = os.path.join(glibc_path, "lib")
assert os.path.isdir(ld_path)
ld_path = os.path.join(glibc_path, "ld-linux-x86-64.so.2")
assert os.path.isfile(ld_path)
assert os.access(ld_path, os.X_OK)

assert os.path.isfile(exe_path)
assert os.access(exe_path, os.X_OK)

cmd = "patchelf --set-interpreter {} --set-rpath {} {}".format(
    ld_path, glibc_path, exe_path)

os.system(cmd)

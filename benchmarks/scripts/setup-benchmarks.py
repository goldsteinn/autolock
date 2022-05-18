#! /usr/bin/env python3

import os
import sys
import subprocess
import argparse

def setgcc(version):
    global gcc_v
    global gxx_v
    rc = subprocess.call(["which", "gcc-{}".format(version)])
    if rc == 0:
        gcc_v = "gcc-{}".format(version)
        gxx_v = "g++-{}".format(version)
        return
    raise ValueError("No version of gcc matches: {}".format(version))
    

# libmemcached needs 7 <= gcc <= 9. Try to find it. Otherwise just system gcc
gcc_v = "gcc"
gxx_v = "g++"
for i in range(9, 6, -1):
    try:
        setgcc(i)
        break
    except ValueError as e:
        continue

################################################################
# setup arguments
parser = argparse.ArgumentParser(description='download and setup benchmarks')
parser.add_argument("-v", "--verbosity", action="store_true", default=False, help="increase output verbosity")
parser.add_argument("-l", "--listbenchs", action="store_true", default=False, help="list bench names")
parser.add_argument("-g", "--forcegcc", default=0, type=int, help="force using this gcc version")
parser.add_argument("-d", "--linuxdir", default="../linux-dev/src", help="directory where to find linux source")
parser.add_argument("bench", nargs="?", help="the bench to install (or all if none listed)")
flags = parser.parse_args()

verbose = flags.verbosity
listonly = flags.listbenchs
linux_src = flags.linuxdir
benchsToInstall={}
doall = False
if flags.bench is None:
    doall = True
else:    
    benchsToInstall[flags.bench] = 1
if flags.forcegcc > 0:
    setgcc(flags.forcegcc)

linux_src = os.path.realpath(linux_src)
assert os.path.isdir(linux_src)

install_prefix = os.path.join(linux_src, "bench-install")
if not os.path.isdir(install_prefix):
    assert os.system("mkdir -p {}".format(install_prefix)) == 0

tmp_path = ".tmp"
init_files = set()
os.system("mkdir -p {}".format(tmp_path))
for f in os.listdir(tmp_path):
    init_files.add(f)

def find_new_file():
    new_files = []
    for f in os.listdir(tmp_path):
        if f not in init_files:
            new_files.append(f)
    assert len(new_files) == 1
    return new_files[0]


class Benchmark():
    def __init__(self, url, outname, configure, make):
        self.url = url
        self.outname = outname

        self.configure = configure
        self.make = make

    def getName(self):
        return self.outname
    
    def tar_path(self):
        return "{}.tar.gz".format(self.outname)

    def src_dir_name(self):
        return "src/{}".format(self.outname)

    def patch_dir_name(self):
        return "patches/{}".format(self.outname)

    def download(self):
        if verbose:
            print("Checking {} and {}".format(self.src_dir_name(), self.tar_path()))
        if os.path.isdir(self.src_dir_name()) or os.path.isfile(
                self.tar_path()):
            return
        wget = os.system("wget {} -O {}".format(self.url,
                                                self.tar_path()))
        if wget == 0:
            return
        # should delete the file/directory if we get here
        print("Failed to download {} into {}, so you should delete".format(self.src_dir_name(), self.tar_path()))
        assert wget == 0


    def unpack(self):
        if os.path.isdir(self.src_dir_name()):
            return
        assert os.system("tar -xzf {} -C {}".format(self.tar_path(),
                                                    tmp_path)) == 0

        fname = find_new_file()
        assert os.system("mv {} {}".format(os.path.join(
            tmp_path, fname), self.src_dir_name())) == 0, "mv {} {}".format(
                os.path.join(tmp_path, fname), self.src_dir_name())

        if not os.path.isdir(self.patch_dir_name()):
            return

        patch_files = []
        for patch_file in os.listdir(self.patch_dir_name()):
            if patch_file.endswith(".patch"):
                patch_files.append(
                    os.path.join(os.path.realpath(self.patch_dir_name()),
                                 patch_file))
        patch_files.sort()
        for patch_file in patch_files:
            assert os.system("(cd {}; patch -p0 < {})".format(
                self.src_dir_name(), patch_file)) == 0

    def cleanup(self):
        if not os.path.isfile(self.tar_path()):
            return
        assert os.system("rm -rf {}".format(self.tar_path())) == 0

    def build(self):
        cmd = "(cd {};".format(self.src_dir_name())
        if self.make is None:
            return

        if self.configure is not None:
            cmd += "{} &&".format(self.configure)

        cmd += " {})".format(self.make)

        assert os.path.isdir(self.src_dir_name())
        assert os.system(cmd) == 0

    def get(self):
        self.download()
        self.unpack()
        self.cleanup()


benchmarks = [
    Benchmark("http://parsec.cs.princeton.edu/download/3.0/parsec-3.0.tar.gz",
              "parsec-3.0", None, None),
    Benchmark("https://github.com/liuyix/splash2_benchmark/tarball/master",
              "splash-2", None, None),
    Benchmark(
        "https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz",
        "libmemcached-1.0.18",
        "export CC={}; export CXX={}; ./configure --enable-memaslap --prefix={}; unset CC; unset CXX"
        .format(gcc_v, gxx_v, install_prefix), "make && make install"),
    Benchmark("http://memcached.org/latest", "memcached-latest",
              "./configure --prefix={}".format(install_prefix),
              "make && make install")
]

for benchmark in benchmarks:
    if listonly:
        print(benchmark.getName())
        continue
    if doall or benchmark.getName() in benchsToInstall:
        print("Setting up {}".format(benchmark.getName()))
        benchmark.get()
        benchmark.build()

if len(init_files) == 0:
    os.system("rm -rf {}".format(tmp_path))

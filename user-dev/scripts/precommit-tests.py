#! /usr/bin/env python3

import os
import sys
import subprocess


class test_cmd():
    def __init__(self,
                 shell_cmd,
                 stdout_scan=-1,
                 stderr_scan=None,
                 expec_return=0):
        self.shell_cmd = shell_cmd
        self.stdout_scan = stdout_scan
        self.stderr_scan = stderr_scan
        self.expec_return = expec_return

    def check_scan(self, scan, data):
        if isinstance(scan, int) and scan == -1:
            return True
        if scan is None or scan == "":
            if not (data is None or data == ""):
                return False
        else:
            if (data is not None) and (scan in data):
                return False
        return True

    def check_results(self, ret, stdout_data, stderr_data):
        if ret != self.expec_return:
            return False
        if not self.check_scan(self.stdout_scan, stdout_data):
            return False
        if not self.check_scan(self.stderr_scan, stderr_data):
            return False
        return True

    def get_cmd(self):
        return self.shell_cmd


def run_process(cmd):
    try:
        sproc = subprocess.Popen(cmd,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)

        stdout_data, stderr_data = sproc.communicate()
        stdout_data = stdout_data.decode("utf-8", "ignore")
        stderr_data = stderr_data.decode("utf-8", "ignore")

        ret = sproc.returncode
        return ret, stdout_data, stderr_data
    except:
        return -1, None, None


def find_path_to_build(cur_path):
    if os.path.basename(cur_path) == "build":
        return cur_path
    if "build" in cur_path.split("/"):
        return find_path_to_build(os.path.split(cur_path)[0])
    else:
        for next_dir in os.listdir(cur_path):
            if next_dir == ".git":
                continue
            next_path = os.path.join(cur_path, next_dir)
            if not os.path.isdir(next_path):
                continue
            next_path = find_path_to_build(next_path)
            if next_path is not None:
                return next_path
        return None


def verify_cmd(path, cmd):
    print("$> {}".format(cmd.get_cmd()))
    exec_cmd = "(cd {}; {})".format(path, cmd.get_cmd())
    ret, stdout_data, stderr_data = run_process(exec_cmd)

    if not cmd.check_results(ret, stdout_data, stderr_data):
        print("Error Running: \"{}\" -> \n{}\n{}\n{}".format(
            exec_cmd, ret, stdout_data, stderr_data))


path = os.getcwd()
if len(sys.argv) > 1:
    path = sys.argv[1]

path = find_path_to_build(path)

extra_flags = ["-DWITH_MATH={}", "-DWITH_THREAD={}", "-DWITH_VDSO={}"]
for i in range(0, (1 << len(extra_flags))):
    flags = []
    for j in range(0, len(extra_flags)):
        v = 0
        if (i & (1 << j)) != 0:
            v = 1
        flags.append(extra_flags[j].format(v))
    flags = " ".join(flags)
    for lang in [["C", "gcc"], ["CXX", "gcc"], ["C", "clang"]]:
        tst_cmds = [
            test_cmd("cmake -DLANG={} -DCOMPILER={} {} ..".format(
                lang[0], lang[1], flags)),
            test_cmd("make clean"),
            test_cmd("make full"),
            test_cmd("./bench --all"),
            test_cmd("./tests --all"),
            test_cmd("make static-analysis",
                     os.path.split(path)[0], "Error")
        ]
        for tst_cmd in tst_cmds:
            verify_cmd(path, tst_cmd)

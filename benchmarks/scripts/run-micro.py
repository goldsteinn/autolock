import sys
import time
import os
import psutil
import subprocess

TIMEOUT = 30
NUM_CPUS = int(psutil.cpu_count() / 2)
PRINTED_HDR = False
BASE_CMD = "sudo ./qemu-script.sh r \"./user-bin/driver {} --csv --outer-iter {} --cs-iter {} --extra-iter {} -t {} --bench  -T {} {} > .tmp-result-out\""

locks = [
    "pthread_mutex", "pthread_spinlock", "spinlock", "backoff_spinlock",
    "yield_spinlock", "yield_backoff_spinlock", "futex_spinlock",
    "futex_backoff_spinlock", "futex_yield_spinlock",
    "futex_yield_backoff_spinlock", "folly_mutex", "abseil_mutex",
    "simple_autolock", "backoff_autolock", "clh_normlock", "clh_autolock",
    "rseq_autolock", "aepfl_autolock", "aepfl_normlock"
]

locks_todo = [
    "pthread_mutex",
    #    "pthread_spinlock",
    "clh_normlock",
    "clh_autolock",
    "aepfl_normlock",
    "aepfl_autolock",
    "cptltkt_normlock",
    "cptltkt_autolock",
    "spin_normlock",
    "spin_autolock",
    #    "yield_normlock",
    #    "yield_autolock",
    "backoff_normlock",
    "backoff_autolock",
    "backoff_yield_normlock",
    "backoff_yield_autolock",
    "backoff_yield_futex_normlock",
    "backoff_yield_futex_autolock",
    "backoff_futex_normlock",
    "backoff_futex_autolock",
    #    "futex_normlock",
    #    "futex_autolock",
    #    "yield_futex_normlock",
    #    "yield_futex_autolock",
]

locks_todo = [
    #    "pthread_spinlock",
    "clh_normlock",
    "clh_autolock",
    "cptltkt_normlock",
    "cptltkt_autolock",
    #    "yield_normlock",
    #    "yield_autolock",
    "backoff_normlock",
    "backoff_autolock",
    "backoff_yield_normlock",
    "backoff_yield_autolock",
    "backoff_futex_normlock",
    "backoff_futex_autolock",
    #    "futex_normlock",
    #    "futex_autolock",
    #    "yield_futex_normlock",
    #    "yield_futex_autolock",
]


RESULT_LEN = -1
RESULT_HDR = None
RUN_HDR = None
OUT = []

CNT = 0

OUTFILE = open("benchmark.log", "w+")

def myprint(msg):
    print(msg)
    OUTFILE.write(msg + "\n")
    OUTFILE.flush()
    
def cpus_to_hex(cpus):
    global NUM_CPUS
    if len(cpus) == 0:
        return hex((1 << NUM_CPUS) - 1)
    cmap = 0
    for cpu in cpus:
        cmap |= (1 << cpu)
    return hex(cmap)


def run_process(cmd):
    global TIMEOUT
    global CNT
    os.system("rm -f .tmp-result-out")
    try:
        myprint(cmd)
        time.sleep(1)
        ts_start = time.time_ns()
        cpu_start = psutil.cpu_percent()

        sproc = subprocess.Popen(cmd,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL)

        stdout_data = sproc.communicate(timeout=TIMEOUT)

        ts_end = time.time_ns()
        cpu_end = psutil.cpu_percent()
        try:
            stdout_data_f = open(".tmp-result-out", "r")
            stdout_data = stdout_data_f.read()
        except IOError:
            myprint("IO Error!")
            return None, [None, None], [None, None]

        myprint(str(stdout_data))
        sys.stdout.flush()
        return stdout_data, [ts_start, ts_end], [cpu_start, cpu_end]

    except subprocess.TimeoutExpired:
        os.system("kill -9 $(pidof qemu-system-x86_64)")
        return None, [None, None], [None, None]


class Result():

    def __init__(self, outer_iter, nthread, data, interval, cpu_usage):
        self.outer_iter = outer_iter
        self.nthread = nthread
        self.data = data.lstrip().rstrip()

        self.interval = interval
        self.cpu_usage = cpu_usage

    def get_outer_iter(self):
        return self.outer_iter

    def get_nthread(self):
        return self.nthread

    def get_hdr(self):
        pieces = self.data.split("\n")
        if len(pieces) != 2:
            return ["ERROR"] * RESULT_LEN
        hdr = pieces[0].lstrip().rstrip().split(",")
        return hdr

    def get_val(self):
        global RESULT_LEN
        pieces = self.data.split("\n")
        if len(pieces) != 2:
            if RESULT_LEN == -1:
                return ["ERROR -- THIS CSV LIVE IS CORRUPTED!"]
            return ["ERROR"] * RESULT_LEN

        val = pieces[1].lstrip().rstrip().split(",")
        if RESULT_LEN == -1:
            RESULT_LEN = len(val)

        return val


def result_to_csv(result):
    global RESULT_HDR
    if result is None:
        return ["ERROR"] * (RESULT_LEN + 6)
    else:
        if RESULT_HDR is None:
            RESULT_HDR = [
                "outer_iter", "nthreads", "ns_start", "ns_end",
                "cpu_usage_start", "cpu_usage_end"
            ] + result.get_hdr()

        return [result.get_outer_iter(),
                result.get_nthread()
                ] + result.interval + result.cpu_usage + result.get_val()


class Run():

    def __init__(self, lock, cpus, outer_iter, cs_iter, extra_iter, trials):
        self.lock = lock
        self.cpus = cpus
        self.outer_iter = outer_iter
        self.cs_iter = cs_iter
        self.extra_iter = extra_iter
        self.trials = trials
        self.results = {}

    # hard codes maximum of 3 threads per core
    def max_nthreads(self):
        global NUM_CPUS
        if len(self.cpus) == 0:
            return NUM_CPUS
        return len(self.cpus) * 4

    def min_nthreads(self):
        return max(len(self.cpus), 1)

    def custom_thread_iter(self):
        return [len(self.cpus), 2 * len(self.cpus), 4 * len(self.cpus)]

    def thread_iter(self):
        global NUM_CPUS

        if True:
            return self.custom_thread_iter()

        iter_set = []

        iter_set += [
            x for x in range(self.min_nthreads(),
                             min(self.min_nthreads() + 4, self.max_nthreads()))
        ]

        iter_set += [
            x * 8 for x in range(self.min_nthreads(), self.max_nthreads())
        ]

        iter_set_f = set()
        iter_set_out = []
        for x in iter_set:
            if x < self.min_nthreads() or x > self.max_nthreads(
            ) or x in iter_set_f:
                continue
            iter_set_f.add(x)
            iter_set_out.append(x)

        iter_set_out.sort()

        return iter_set_out

    def is_pinned(self):
        r = 0
        if len(self.cpus) != 0:
            r = 1
        return r

    def cpus_arg(self):
        if len(self.cpus) == 0:
            return ""
        else:
            return "--cpus {}".format(",".join(map(str, self.cpus)))

    def run(self):
        div = 25
        for i in self.thread_iter():
            result = None
            oiter = self.outer_iter
            while True:
                data, interval, cpu_usage = run_process(self.cmd(i, oiter))
                if data is not None:
                    result = Result(oiter, i, data, interval, cpu_usage)
                    break
                if oiter <= (10 * 1000):
                    break
                oiter = int(oiter / div)

            self.results[i] = result

    def cmd(self, threads, oiter):
        global BASE_CMD

        return BASE_CMD.format(self.cpus_arg(), oiter, self.cs_iter,
                               self.extra_iter, self.trials, threads,
                               self.lock)

    def fmt_info(self):
        return [
            self.lock,
            cpus_to_hex(self.cpus),
            self.is_pinned(), self.cs_iter, self.extra_iter, self.trials
        ]

    def fmt_hdr(self):
        return [
            "lock", "cpu_mask", "is_pinned", "cs_iter", "extra_iter", "trials"
        ]

    def display(self):
        global OUT
        global RUN_HDR
        if RUN_HDR is None:
            RUN_HDR = self.fmt_hdr()

        for i in self.results:
            OUT.append(",".join(
                map(str,
                    self.fmt_info() + result_to_csv(self.results[i]))))


# usable cpu list for pinning
cpu_confs = []
i = 1
while i <= min(8, NUM_CPUS):
    cpu_confs.append([x for x in range(0, i)])
    i += i

while i <= min(48, NUM_CPUS):
    if i == 32:
        i += i
        continue
    cpu_confs.append([x for x in range(0, int(i / 2))] +
                     [24 + x for x in range(0, int(i / 2))])
    i += i

trials_conf = 5

# total times you grab lock
outer_iter_conf = 25 * 100 * 1000
# critical section length
cs_iter_confs = [1, 10, 100]
# other work besides lock
extra_iter_confs = [1, 10, 100]

runs = []
for cpu_conf in cpu_confs:
    for cs_iter_conf in cs_iter_confs:
        for extra_iter_conf in extra_iter_confs:
            iters = cs_iter_conf + extra_iter_conf
            if iters == 110:
                continue
            
            for lock in locks_todo:
                runs.append(
                    Run(lock, cpu_conf, outer_iter_conf, cs_iter_conf,
                        extra_iter_conf, trials_conf))

for run in runs:
    run.run()

for run in runs:
    run.display()


myprint(",".join(RUN_HDR + RESULT_HDR))
myprint("\n".join(OUT))

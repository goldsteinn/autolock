import subprocess

TIMEOUT = 15
PRINTED_HDR = False
BASE_CMD = "./user-bin/driver {} --csv-out --outer-iter {} --cs-iter {} --extra-iter {} -t {} --bench  -T {} {}"

locks = [
    "pthread_mutex", "pthread_spinlock", "spinlock", "backoff_spinlock",
    "yield_spinlock", "yield_backoff_spinlock", "futex_spinlock",
    "futex_backoff_spinlock", "futex_yield_spinlock",
    "futex_yield_backoff_spinlock", "folly_mutex", "abseil_mutex",
    "simple_autolock", "backoff_autolock", "clh_normlock", "clh_autolock",
    "rseq_autolock", "aepfl_autolock", "aepfl_normlock"
]

locks_todo = [
    "pthread_mutex", "backoff_spinlock", "yield_backoff_spinlock",
    "futex_backoff_spinlock", "simple_autolock", "backoff_autolock",
    "clh_normlock", "clh_autolock"
]

RESULT_LEN = -1
RESULT_HDR = None
RUN_HDR = None
OUT = []

CNT = 0


def cpus_to_hex(cpus):
    if len(cpus) == 0:
        return hex(0xf)
    cmap = 0
    for cpu in cpus:
        cmap |= 1
    return hex(cmap)


def run_process(cmd):
    global TIMEOUT
    global CNT
    try:
        print(cmd)
        sproc = subprocess.Popen(cmd,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL,
                                 timeout=TIMEOUT)

        stdout_data = sproc.communicate()
        return stdout_data[0]

    except TimeoutError:
        return None


class Result():

    def __init__(self, outer_iter, nthread, data):
        self.outer_iter = outer_iter
        self.nthread = nthread
        self.data = data.lstrip().rstrip()

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
        return ["ERROR"] * (RESULT_LEN + 2)
    else:
        if RESULT_HDR is None:
            RESULT_HDR = ["outer_iter", "nthreads"] + result.get_hdr()

        return [result.get_outer_iter(),
                result.get_nthread()] + result.get_val()


class Run():

    def __init__(self, lock, cpus, outer_iter, cs_iter, extra_iter, trials):
        self.lock = lock
        self.cpus = cpus
        self.outer_iter = outer_iter
        self.cs_iter = cs_iter
        self.extra_iter = extra_iter
        self.trials = trials
        self.results = {}

    def max_nthreads(self):
        if len(self.cpus) == 0:
            return 12
        return len(self.cpus) * 3

    def min_nthreads(self):
        return max(len(self.cpus), 1)

    def thread_iter(self):
        return [x for x in range(self.min_nthreads(), self.max_nthreads())]

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

        for i in self.thread_iter():
            result = None
            oiter = self.outer_iter
            while True:
                data = run_process(self.cmd(i, oiter))
                if data is not None:
                    result = Result(oiter, i, data)
                    break
                if oiter <= (10 * 1000):
                    print("\tError")
                    break
                oiter = int(oiter / 25)

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


cpu_confs = [[], [0], [0, 1], [0, 1, 2], [0, 1, 2, 3]]
trials_conf = 10

outer_iter_conf = 10 * 1000 * 1000
cs_iter_confs = [1, 25]
extra_iter_confs = [1, 25]

runs = []
for cpu_conf in cpu_confs:
    for cs_iter_conf in cs_iter_confs:
        for extra_iter_conf in extra_iter_confs:
            for lock in locks_todo:
                runs.append(
                    Run(lock, cpu_conf, outer_iter_conf, cs_iter_conf,
                        extra_iter_conf, trials_conf))

for run in runs:
    run.run()

for run in runs:
    run.display()

print(",".join(RUN_HDR + RESULT_HDR))
print("\n".join(OUT))

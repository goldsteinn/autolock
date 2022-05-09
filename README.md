# Autolock


## Setup

**Note: All commands are written under the expectation they are done
from the toplevel directory (where this `README` is)**

### Linux Install Dependencies

```
sudo apt install build-essentialfakeroot libncurses5-dev libssl-dev ccache flex bison libelf-dev
```

### Qemu Install Dependencies

```
$> sudo apt install dwarves busybox qemu-system-x86
```

### User-dev Install Dependencies

```
# Ubuntu 22+
$> sudo apt install libdouble-conversion-dev libfmt-dev libglog-dev libunwind-dev libboost-all-dev
# Ubuntu 18
$> sudo apt install libdouble-conversion-dev libfmt-dev libgoogle-glog-dev libunwind-dev libboost-all-dev
```

**After this point NOTHING should require `sudo`.**


### Linux Setup

1. **Get Linux**
    ```
    $> git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git linux-dev/src
    ```
2. **Checkout 5.18-rc4**
    ```
    $> (cd linux-dev/src; git checkout v5.18-rc4; git checkout -b autolock-base)
    ```

3. **Apply autolock patches**
    ```
    $> (cd linux-dev/src; git am ../../patches/*)
    ```

4. **Config Linux**
    ```
    # Note: there is an example config in `linux-dev/config/example-config`.
    # Some difference are expected based on compiler version / exact system
    # info but any config difference related to virtualization, serial ports,
    # or scheduling will be an issue.
    # see: https://ops.tips/notes/booting-linux-on-qemu/ and https://www.linux-kvm.org/page/Virtio if any issues.
    $> (cd linux-dev/src; make x86_64defconfig)
    ```

5. **Build linux**
    ```
    $> (cd linux-dev/src; make)
    ```

6. **Copy `qemu` script to linux source directory**
    ```
    $> cp linux-dev/scripts/qemu-script.sh linux-dev/src
    ```

    This is annoying but the script must be run from the linux source tree


### Userland Setup

1. **Init all submodules**
    ```
    # Note: At the moment there are no submodules but we may want to
    # test against `abseil`/`folly` implementations.
    $> git submodule update --init --recursive
    ```

2. **Config userland support**
    ```
    $> cmake -DBUILD_TESTING=OFF -DLANG=CXX -DCOMPILER=g++ -DWITH_MATH=1 -DWITH_THREAD=1 -DWITH_VDSO=1 -S user-dev/ -B user-dev/build
    ```

3. **Build userland support**
    ```
    # Generally best workflow is keep a seperate shell around for
    # rebuilding / installing
    $> (cd user-dev/build; make)
    ```

4. **Install userland support**
    ```
    # Note: We install this into the kernel source folder so we don't
    # need to change directories after entering the virtual
    # machine. This installs to `linux-dev/src/user-bin`
    $> (cd user-dev/build; make install)
    ```

## Running

1. **Enter Linux source directory**
    ```
    $> cd linux-dev/src
    ```

2. **Run `qemu`**
    ```
    # If this fails try re-configuring/building and ensure following are set
    # CONFIG_VIRTIO_PCI=y
    # CONFIG_VIRTIO_BALLOON=y
    # CONFIG_VIRTIO_BLK=y
    # CONFIG_VIRTIO_NET=y
    # CONFIG_VIRTIO=y
    # CONFIG_VIRTIO_RING=y
    # use `make menuconfig`.
    # Don't run `make menuconfig` from an emacs shell. Use bash.
    # You can search by typing '/' then 'VIRTIO_<config_postifx>'
    $> ./qemu-script.sh
    ```

3. **Run tests**
    ```
    $> ./user-bin/driver --test --all
    ```

4. **Run benchmarks**
    ```
    $> ./user-bin/driver --bench --all
    ```

5. **Kill qemu**
    ```
    # If no errors you can usually do Ctrl-D
    $> kill -9 $(pidof qemu-system-x86_64)
    ```

## Browsing

### Kernel Code

To browse the kernel code get to step 3 in the [`Linux Setup`](#linux-setup)

The autlock commits start just after the `Linux 5.18-rc4` merge:

```
commit af2d861d4cd2a4da5137f795ee3509e6f944a25b (tag: v5.18-rc4)
Author: Linus Torvalds <torvalds@linux-foundation.org>
Date:   Sun Apr 24 14:51:22 2022 -0700

    Linux 5.18-rc4
```


So anything since then is part of autolock development.

The files of interest ordered as such

1. **Autolock core**
    - Core autolock implementation: `kernel/auto-lock.c`
    - Autolock kernel level API: `include/linux/auto-lock.h`
    - Autolock user level API: `include/uapi/linux/auto-lock.h`

2. **Autolock scheduler integration**
    - Autolock integration into CFS: `kernel/sched/fair.c`
    - Autolock integration into context switch: `kernel/sched/core.c`

3. **Autolock task integration**
    - Integration into `task_struct`: `include/linux/sched.h`
    - Integration into task teardown: `kernel/fork.c`

4. **Autolock syscalls**
    - `arch/x86/entry/syscalls/syscall_64.tbl`
    - `include/linux/syscalls.h`
    - `include/uapi/asm-generic/unistd.h`
    - `kernel/sys_ni.c`

5. **Other stuff for logging / building**
    - `kernel/sched/deadline.c`
    - `kernel/sched/idle.c`
    - `kernel/sched/rt.c`
    - `include/linux/auto-lock-verbose.h`
    - `kernel/Makefile`



### User Code

99% of the code in this project is boiler plate I carry with me. As a
general note any symbol that has the prefix `I_*` is explicitly not
part of any external API and is meant for internal use only. Any
symbol without that prefix is meant to be part of the exported API.

Finally, note that the the two targets [`test-driver`] and
[`bench-driver`] are only for testing / benchmarking internal
functionality of the suite and are **entirely unrelated to the
`autolock` project.**

The two directories that have interesting stuff are [`src/autolock-impls`](https://github.com/goldsteinn/autolock/tree/master/user-dev/src/autolock-impls) and [`src/locks`](https://github.com/goldsteinn/autolock/tree/master/user-dev/src/locks).


#### Exported API

    - All locks are exported as a **C++ `class`** with the same API. An example can be seen with with [`pthread_mutex`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L57).
    - The API includes:
        1. `static __typeof__(this) init(void *) /* __typeof__(this) is a pointer to the class type. */ `
        2. `void destroy()`
        3. `int try_lock()`
        4. `void lock()`
        5. `void unlock()`
    - The API is used by the templated benchmark code in [`src/locks/lock-bench.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-bench.h).


#### Autolock Implementations

Found in directory: [`src/autolock-impls/`](https://github.com/goldsteinn/autolock/tree/master/user-dev/src/autolock-impls)


1. **Autolock ABI for interacting with kernel shared memory**
    - [`src/autolock-impls/autolock-abi.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/autolock-abi.h) defines the memory layout.
    - [`src/autolock-impls/kernel-autolock.c`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/kernel-autolock.c) defines the TLS storage for the userland pointer to the shared mapping.
    - Neither of these should be included directly.

2. **Autolock API for interacting with the kernel**
    - [`src/autolock-impls/autolock-api.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/autolock-api.h) This includes setup/teardown and the `extern` def for the TLS storage.
    - This is included by all autolock implementations.

3. **Autolock exported API**
    - [`src/autolock-impls/autolock-export.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/autolock-export.h) defines all the user-level locks that use the kernel autolock feature. This is included by the [general purpose lock declaration code](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L4).
    - The class structure is just the general API that all tested locks will adhere too.
    - The exact list of exported implementations is set at [`#define AUTOLOCK_IMPLS`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/autolock-export.h#L41)

4. **Actual user-level autolocks**
    - [`src/autolock-impls/simple-autolock.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/autolock-impls/simple-autolock.h) Just a simple spinlock.


#### Other reference locks

1. **All lock declarations**
    - all lock implementations are declared in [`src/locks/locks-decls.c`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c). They are declared as a [`decl_list_t`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/util/func-decl-types.h#L21) which is essentially a statically sized array of function pointers + names.
    - All implementations are defined in the list [`#define LOCK_IMPLS`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L122).

2. **Current lock implementations (aside from autolocks)**
    - [`pthread_mutex`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L57)
    - [`pthread_spinlock`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L90)
    - [a variety of configs built ontop of `src/locks/lock-base.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-decls.c#L44). These are just references to help see what features matter for performance.


#### Bench/Test Code

1. **Low level bench/test functions**
    - The internal bench/test code that is templated for each lock implementation is in [`src/locks/lock-bench.h`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-bench.h).
    - The API used for the thread function is [`void * bench_runner(void *)`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-bench.h#L51).
    - The low level benchmark function is [`I_bench_runner_kernel()`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-bench.h#L18)

2. **High level bench/test driver**
    - The driver code for calling `bench_runner` is in [`src/locks/lock-runner.c`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-runner.c). This will always test that the result is correct. The difference between benchmark/test is that benchmarking will collect and export stats.
    - The API is simply [`run(func_decl_t const *, run_params_t *, stats_result_t *)`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/locks/lock-runner.h#L8)

#### Main Function
- The main function is found in [`src/driver.c`](https://github.com/goldsteinn/autolock/blob/master/user-dev/src/driver.c)
- The general usage is:
    - `$> ./driver --bench --threads <num_threads> --trials <num_trials> <rest=locks to test/bench>`
- See `$> ./driver -h` for all options.
- To run all locks you can use `--all` and to list the available locks you can use `--list`.
- Searching for lock names will essentially use `fnmatch` syntax so something like:
    - `$> ./driver --test spin*` will test all lock implementations whose names are prefixed with 'spin'.

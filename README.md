## Autolock


#### Setup

**Note: All commands are written under the expectation they are done
from the toplevel directory (where this `README` is)**

###### Qemu Install Dependencies

```
$> sudo apt install dwarves
$> sudo apt install busybox
$> sudo apt install qemu-system-x86
```

**After this point NOTHING should require `sudo`.**


###### Linux Setup

1. **Get Linux**
    ```
    $> git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git linux-dev/src
    ```
2. **Checkout 5.18-rc4**
    ```
    $> (cd linux-dev/src; git checkout v5.18-rc4; git checkout -b autolock-base)
    ```

3. **Config Linux**
    ```
    # Note: there is an example config in `linux-dev/config/example-config`.
    # Some difference are expected based on compiler version / exact system
    # info but any config difference related to virtualization, serial ports,
    # or scheduling will be an issue.
    # see: https://ops.tips/notes/booting-linux-on-qemu/ and https://www.linux-kvm.org/page/Virtio if any issues.
    $> (cd linux-dev/src; make x86_64defconfig)
    ```

4. **Apply autolock patches**
    ```
    $> (cd linux-dev/src; git am ../../patches/*)
    ```

5. **Rebuild linux**
    ```
    $> (cd linux-dev/src; make)
    ```

6. **Copy `qemu` script to linux source directory**
    ```
    $> cp linux-dev/scripts/qemu-script.sh linux-dev/src
    ```

    This is annoying but the script must be run from the linux source tree


###### Userland Setup

1. **Init all submodules**
    ```
    # Note: At the moment there are no submodules but we may want to
    # test against `abseil`/`folly` implementations.
    # $> git submodule --init --recursive
    ```

2. **Config userland support**
    ```
    $> cmake -DLANG=CXX -DCOMPILER=g++ -DWITH_MATH=1 -DWITH_THREAD=1 -DWITH_VDSO=1 -S user-dev/ -B user-dev/build
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

#### Running

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

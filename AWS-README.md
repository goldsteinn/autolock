## All Dependencies (No third-party build)

```
sudo apt update
sudo apt install git
sudo apt install gcc g++ g++-9 gcc-9 flex bison
sudo apt install build-essential fakeroot libncurses5-dev libssl-dev ccache flex bison libelf-dev
sudo apt install dwarves busybox qemu-system-x86
sudo apt install patchelf
sudo apt install cmake
sudo apt install libevent-dev
sudo apt install python3-pip
pip3 install psutil
sudo pip3 install psutil # Need for both user and root
```


## Issues

### Loading Qemu

```
qemu-system-x86_64: -watchdog i6300esb: warning: -watchdog is deprecated; use -device instead.
Could not access KVM kernel module: No such file or directory
qemu-system-x86_64: failed to initialize kvm: No such file or directory
qemu-system-x86_64: falling back to tcg
qemu-system-x86_64: CPU model 'host' requires KVM or HVF
```

[Seem to need to load `modprobe kvm-intel`](https://stackoverflow.com/questions/14542754/qemu-kvm-kernel-module-no-such-file-or-directory)
```
root$> modprobe kvm-intel
modprobe: ERROR: could not insert 'kvm_intel': Operation not supported

# Success but doesn't do anything
root$> modprobe kvm
```

[Enabling KVM in ec2 instance is not something we have control over if we don't get baremetal instance](https://stackoverflow.com/questions/65944922/how-to-use-enable-kvm-in-amazon-aws-instances)

[With baremetal instance this is instuctions](https://github.com/aws-samples/aws-bare-metal-kvm-demo)

**To fix this we need baremetal**

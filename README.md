#README
Repository for kernel and userspace module in 3.14 kernel for ARMv7

Steps done as below.
1. Create a repo for kernel module.
2. Follow LKMPG 2.6 documentation to create a kernel module code and user space code.
3. Both are targetted for ARMv7.
4. Cross compiled using arm-linux-gnueabihf-.
5. Kernel module linked on 3.x kernel.
6. Use 3.x specific APIs and highlight obsolete APIs.

make user-> user space.

make -> kernel module.

make clean-> removes both userspace code and kernel module.

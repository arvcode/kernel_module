obj-m +=arm_module.o

KERNELDIR=$(BUILDTOP)/linux
all:
	 make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	${RM} *.o arm_userspace
user:
	${CROSS_COMPILE}gcc -o arm_userspace arm_userland.c 
# obj-m+=startstop.o
# startstop-objs:=start.o stop.o

ifneq ($(KERNELRELEASE),)
	obj-m :=hello.o
else
	KERNELDIR :=/lib/modules/$(shell uname -r)/build
	PWD :=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc user.c -o user
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -rf user *~ *.o *.ko *.mod.c *.mod *.cmd modules.order Module.markers Module.symvers .tmp_versions
endif


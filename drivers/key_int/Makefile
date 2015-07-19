ifneq ($(KERNELRELEASE),)
    obj-m := key_int.o
else
    KERNELDIR ?= /home/can/share/linux-2.6.28.7/
    PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *.ko *.mod.c *.mod.o *.order *.symvers

		

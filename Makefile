obj-m += cpufake_ksu.o
KDIR := /tmp/kernel
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) ARCH=arm64 modules

clean:
	make -C $(KDIR) M=$(PWD) clean

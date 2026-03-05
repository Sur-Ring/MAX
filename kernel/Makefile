TARGET = max_kernel

obj-m += $(TARGET).o

$(TARGET)-y := src/main.o src/netlink.o src/api.o src/shm.o

ccflags-y += -I. -I$(src)/../include -Isrc -I../common -O3 -D__MAX_KERNEL__ -DMODULE_NAME=\"$(TARGET)\"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

load:
	sudo dmesg -c
	sudo insmod $(TARGET).ko
	sudo dmesg -wc | tee dmesg.log

rm:
	sudo rmmod $(TARGET)

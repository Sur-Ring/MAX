TARGET = my_module

obj-m += $(TARGET).o

$(TARGET)-y := ../src/main.o

ccflags-y += -I../src -I../include -I../../../common -I$(src)/../../include -O3 -D__MAX_KERNEL__ -DMODULE_NAME=\"$(TARGET)\" -march=native -mhard-float -msse

KBUILD_EXTRA_SYMBOLS += $(src)/../../kernel/Module.symvers

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

ins:
	sudo insmod $(TARGET).ko dir_path=$(CURDIR)

rm:
	sudo rmmod $(TARGET)

module:
	hipcc --genco ../src/module.hip -o ../src/module.co

test:
	make ins
	make rm
TARGET = my_module

compile:
	make -C build
	cp build/my_module.ko my_module.ko
	cp build/Module.symvers Module.symvers

ins:
	sudo insmod $(TARGET).ko

rm:
	sudo rmmod $(TARGET)

mod:
	hipcc --genco src/module.hip -o src/module.co

test:
	sudo insmod $(TARGET).ko
	sudo rmmod $(TARGET)
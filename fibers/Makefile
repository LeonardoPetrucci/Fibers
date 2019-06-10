obj-m+=fibers.o
fibers-y := module.o device.o fiber.o proc.o probes.o
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean 

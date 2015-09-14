KERNELDIR ?=/opt/FriendlyARM/6410/linux-2.6.38-v1.1.2
all: robort_car 

robort_car : robort_car.c camera.c robort_car.h
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean:
	rm robort_car
















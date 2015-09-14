KERNELDIR ?=/opt/FriendlyARM/6410/linux-2.6.38-v1.1.2
all: robort_controller

robort_controller : robort_controller.c lcd.c lcd.h
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean:
	rm robort_controller
















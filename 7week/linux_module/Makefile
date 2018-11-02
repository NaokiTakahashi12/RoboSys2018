obj-m:= led.o

led.ko: led.c
	make -C /usr/src/linux M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux M=`pwd` V=1 clean

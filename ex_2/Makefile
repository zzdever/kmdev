obj-m+=HelloModule.o

all:
	make -C /usr/src/kernel/ M=$(shell pwd) modules

clean:
	rm *.o *.mod.c clean



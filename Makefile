obj-m += s2fs.o
all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

uninstall: 
	sudo rmmod s2fs.ko 

install: 
	sudo insmod s2fs.ko 

	

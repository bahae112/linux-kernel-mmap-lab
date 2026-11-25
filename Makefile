
# Kernel module
obj-m += mydevice.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CC := gcc
CFLAGS := -Wall -Wextra

.PHONY: all module apps load unload clean

# Compile everything
all: module apps

# Compile kernel module
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Compile user applications
apps: app mmap_app seccomp_app

app: app.c
	$(CC) $(CFLAGS) -o app app.c

mmap_app: mmap_app.c
	$(CC) $(CFLAGS) -o mmap_app mmap_app.c

seccomp_app: seccomp_app.c
	$(CC) $(CFLAGS) -o seccomp_app seccomp_app.c

load:
	sudo insmod mydevice.ko
unload:
	sudo rmmod mydevice
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f app mmap_app seccomp_app


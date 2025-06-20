TARGET_MODULE:=t9kbd
BUILD_DIR:=../build/
# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
	$(TARGET_MODULE)-objs := main.o trie.o wordlist.o
	obj-m := $(TARGET_MODULE).o
# If we running without kernel build system
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)
all : 
	cp ../src/*.c .
	cp ../src/*.h .
# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules
clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
load:
	insmod ./$(TARGET_MODULE).ko
unload:
	rmmod ./$(TARGET_MODULE).ko
endif

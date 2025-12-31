CC = gcc
CFLAGS := -O0 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors \
	-fPIC
CFLAGS += -I ../../Symlib/include -DDYNLINK
SRC_DIR := $(PWD)

MOD_ARGS :=  MODULE_DEST=/root/greeter MODULE_NAME=greeter V=1 FEDORA_RELEASE=43 LINUX_PATH=/home/user/Symbi-OS/linux MODULE_LOCAL_DEST=/home/user/Symbi-OS/built_modules

MODULE_SRC=$(SRC_DIR)/kmod
KERNEL_BUILD_PATH=/home/user/Symbi-OS/linux

all: main main.readelf.txt main_static main_static.readelf.txt main.preproc.c

# kernel module build
greeter.ko: kmod/main.c kmod/interpos.c kmod/common.h
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(MODULE_SRC) V=1 modules
	cp kmod/greeter.ko .

greeter.o: greeter.ko
	ld -r -b binary -o greeter.o greeter.ko


# kmod load logic
load_mod.o: load_mod.c
	$(CC) $(CFLAGS) -c -o load_mod.o load_mod.c


# main program
main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

main: greeter.o load_mod.o main.o libkallsyms.so
	$(CC) $(CFLAGS) -o main greeter.o load_mod.o main.o -lc -L../../Symlib/dynam_build -lSym -L. -lkallsyms 

main_static: greeter.o load_mod.o main.o libkallsyms.a
	$(CC) $(CFLAGS) -o main_static greeter.o load_mod.o main.o libkallsyms.a -lc -L../../Symlib/dynam_build -lSym 



main.readelf.txt:
	readelf -Wa main > main.readelf.txt

main_static.readelf.txt:
	readelf -Wa main_static > main_static.readelf.txt

main.preproc.c: main.c
	$(CC) $(CFLAGS) -E main.c -o main.preproc.c


libkallsyms.so:
	sudo cp /proc/libkallsyms.so .

libkallsyms.a:
	sudo cp /proc/libkallsyms.a .



clean:
	rm -f ifunc_elevate ifunc_e_early i_load_mod i_load_mod_static greeter.o greeter.ko load_mod.o ifunc_elevate.o ifunc_elevate.preproc.c libmylib.so libmylib.nodebug.so libmylib.nosymtab.so mylib.o libkallsyms.* *.readelf.txt
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(MODULE_SRC) clean
.PHONY: all clean
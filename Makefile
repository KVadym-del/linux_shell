SHELLFLAGS ?= -Os -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -static -static-libstdc++
INITFLAGS ?= -Os -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -static

LUA ?= echo lua >> files

all: soft_clean build_iso

shell.o: main.cpp
	g++ -c ${SHELLFLAGS} -o shell.o main.cpp

sys.o: sys.S
	as sys.S -o sys.o

init: shell.o sys.o
	g++ -Os ${INITFLAGS} \
	-Wl,--strip-all \
	-Wl,-z,noexec \
	shell.o sys.o -o init

files: init
	echo init >> files
	${LUA}

init.cpio: files
	cat files | cpio -H newc -o > init.cpio

build_iso: init.cpio
	cp init.cpio ../linux/init.cpio; \
	cd ../linux; \
	make isoimage FDARGS="initrd=/init.cpio" FDINITRD=./init.cpio -j18

run: build_iso
	cd ../linux; \
	qemu-system-x86_64 -cdrom arch/x86/boot/image.iso

soft_clean:
	rm -f init shell.o sys.o
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f *.bin
	rm -f *.elf
	rm -f *.cpio
	rm -f ../linux/init.cpio
	rm -f *.img
	rm -f *.gz
	rm -f files

clean: soft_clean
	cd ../linux; \
	make clean;

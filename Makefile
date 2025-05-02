SHELLFLAGS ?= -Os -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -static -static-libstdc++
INITFLAGS ?= -Os -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -static

BUILDDIR ?= build
BINDIR ?= bin
SRCDIR ?= src

LUA ?= echo lua >> files

all: soft_clean build_iso

ls: ${SRCDIR}/ls.cpp
	g++ ${SHELLFLAGS} -o ${BINDIR}/ls ${SRCDIR}/ls.cpp

shell.o: ${SRCDIR}/shell.cpp
	g++ -c ${SHELLFLAGS} -o ${BUILDDIR}/shell.o ${SRCDIR}/shell.cpp

sys.o: ${SRCDIR}/sys.S
	as ${SRCDIR}/sys.S -o ${BUILDDIR}/sys.o

init: shell.o sys.o
	g++ -Os ${INITFLAGS} \
	-Wl,--strip-all \
	-Wl,-z,noexec \
	${BUILDDIR}/shell.o ${BUILDDIR}/sys.o \
	-o ${BINDIR}/init

files: ls init 
	cd ./bin; \
	echo init >> files; \
	${LUA}; \
	echo ls >> files;

init.cpio: files
	cd ./bin; \
	cat files | cpio -H newc -o > init.cpio

build_iso: init.cpio
	cp ${BINDIR}/init.cpio ../linux/init.cpio; \
	cd ../linux; \
	make isoimage FDARGS="initrd=/init.cpio" FDINITRD=./init.cpio -j18

run: build_iso
	cd ../linux; \
	qemu-system-x86_64 -cdrom arch/x86/boot/image.iso

soft_clean:
	rm -f ${BINDIR}/init ${BUILDDIR}/shell.o ${BUILDDIR}/sys.o
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f *.bin
	rm -f *.elf
	rm -f ${BINDIR}/*.cpio
	rm -f ../linux/init.cpio
	rm -f *.img
	rm -f *.gz
	rm -f ${BINDIR}/files

clean: soft_clean
	cd ../linux; \
	make clean;

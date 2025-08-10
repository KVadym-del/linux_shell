BUILDDIR ?= build
BINDIR ?= bin
SRCDIR ?= src

LUA ?= echo lua >> files;
VIM ?= echo vim >> files;

all: soft_clean build_iso

dist_build: 
	cd src && $(MAKE) all

files: dist_build
	cd ./bin; \
	echo init >> files; \
	${LUA} \
	${VIM} \
	echo cat >> files; \
	echo edit >> files; \
	echo ls >> files; \
	echo mkdir >> files; \
	echo touch >> files; \
	echo rm >> files

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
	rm -f ${BINDIR}/edit ${BINDIR}/ls ${BINDIR}/mkdir ${BINDIR}/init ${BINDIR}/*.cpio ${BINDIR}/files
	rm -f ${BUILDDIR}/shell.o ${BUILDDIR}/sys.o
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f *.bin
	rm -f *.elf
	rm -f ../linux/init.cpio
	rm -f *.img
	rm -f *.gz

clean: soft_clean
	cd ../linux; \
	make clean;

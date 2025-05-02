INITFLAGS ?= -O3 -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -static
SHELLFLAGS ?= ${INITFLAGS} -static-libstdc++

BUILDDIR ?= ../build
BINDIR ?= ../bin

all: ls mkdir touch rm init

ls: ls.cpp
	g++ ${SHELLFLAGS} -o ${BINDIR}/ls ls.cpp

mkdir: mkdir.cpp
	g++ ${SHELLFLAGS} -o ${BINDIR}/mkdir mkdir.cpp

touch: touch.cpp
	g++ ${SHELLFLAGS} -o ${BINDIR}/touch touch.cpp

rm: rm.cpp
	g++ ${SHELLFLAGS} -o ${BINDIR}/rm rm.cpp

shell.o: shell.cpp
	g++ -c ${SHELLFLAGS} -o ${BUILDDIR}/shell.o shell.cpp

sys.o: sys.S
	as sys.S -o ${BUILDDIR}/sys.o

init: shell.o sys.o
	g++ -O3 ${INITFLAGS} \
	-Wl,--strip-all \
	-Wl,-z,noexec \
	${BUILDDIR}/shell.o ${BUILDDIR}/sys.o \
	-o ${BINDIR}/init
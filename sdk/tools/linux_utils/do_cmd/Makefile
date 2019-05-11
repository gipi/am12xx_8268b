CFLAGS=-g -Wall
CC=mips-linux-gnu-gcc -EL
AR=mips-linux-gnu-ar
RANLIB=mips-linux-gnu-ranlib
LIBS=-L./ -lsoket_opt

all: libsoket_opt.a do_cmd 

release: all
	cp do_cmd ../../../rootfs/usr/bin/

install: do_cmd
	cp do_cmd ../../../rootfs/usr/bin/
	@echo install do_cmd to ../../../rootfs/usr/bin/.

libsoket_opt.a: soket_opt.o soket_opt.h
	rm -f libsoket_opt.a
	$(AR) rc libsoket_opt.a soket_opt.o
	$(RANLIB) libsoket_opt.a

#mingw32 and cygwin users: replace .cgi with .exe
do_cmd: do_cmd.o libsoket_opt.a
	$(CC) do_cmd.o -o do_cmd ${LIBS}

clean:
	rm -f *.o *.a do_cmd
	rm -f ../../../rootfs/usr/bin/do_cmd

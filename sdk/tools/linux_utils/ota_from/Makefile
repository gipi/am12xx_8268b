CFLAGS=-g -Wall
CC=mips-linux-gnu-gcc -EL
AR=mips-linux-gnu-ar
RANLIB=mips-linux-gnu-ranlib
LIBS=-L./ -lsoket_opt

all: libsoket_opt.a ota_from 

release: all
	cp ota_from ../../../rootfs/usr/

install: ota_from
	cp ota_from ../../../rootfs/usr/
	@echo install ota_from to ../../../rootfs/usr/.

libsoket_opt.a: soket_opt.o soket_opt.h
	rm -f libsoket_opt.a
	$(AR) rc libsoket_opt.a soket_opt.o
	$(RANLIB) libsoket_opt.a

#mingw32 and cygwin users: replace .cgi with .exe
ota_from: ota_from.o libsoket_opt.a
	$(CC) ota_from.o -o ota_from ${LIBS}

clean:
	rm -f *.o *.a ota_from
	rm -f ../../../rootfs/usr/ota_from

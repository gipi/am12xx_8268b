CFLAGS=-g -Wall
CC=gcc
AR=ar
RANLIB=ranlib
LIBS:=-L./ -lcgic  
all: libcgic.a   websetting.cgi


release: CC=mips-linux-gnu-gcc -EL  -muclibc -msoft-float
release: AR=mips-linux-gnu-ar
release: RANLIB=mips-linux-gnu-ranlib
release: all
	cp -f *.cgi ../../../Flash/EZCast_smaller/patch_ezwire_snor/thttpd/html/cgi-bin

install: libcgic.a
	cp libcgic.a /usr/local/lib
	cp cgic.h /usr/local/include
	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libcgic.a: cgic.o cgic.h 
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o 
	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe



websetting.cgi: websetting.o libcgic.a
	$(CC) websetting.o -o websetting.cgi ${LIBS}





clean:
	rm -f *.o *.a *.cgi 

CFLAGS=-g -Wall
CC=mips-linux-gnu-gcc -EL
AR=mips-linux-gnu-ar
RANLIB=mips-linux-gnu-ranlib
LIBS:=-L../../../../case/lib -lamsocket 
INCS:=-I../../../../case/library/socket
CFLAGS+=${INCS}

all:  snmp_am

release: all
	cp snmp_am ../../../rootfs/usr/sbin/

install: snmp_am
	cp snmp_am ../../../rootfs/usr/sbin/
	@echo install snmp_am to ../../../rootfs/usr/sbin/.



#mingw32 and cygwin users: replace .cgi with .exe
snmp_am: snmp_am.o
	$(CC) snmp_am.o -o snmp_am ${LIBS} 


clean:
	rm -f *.o *.a snmp_am
	rm -f ../../../rootfs/usr/sbin/snmp_am
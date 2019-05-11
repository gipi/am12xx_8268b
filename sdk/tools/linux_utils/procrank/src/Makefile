CFLAGS=-g -Wall
CROSS=mips-linux-gnu
CC=$(CROSS)-gcc -EL
STRIP=$(CROSS)-strip

all: procrank

procrank: procrank.c pm_kernel.c pm_map.c pm_memusage.c pm_process.c pagemap.h pm_map.h
	$(CC) $(CFLAGS) procrank.c pm_kernel.c pm_map.c pm_memusage.c pm_process.c -o procrank 
	$(STRIP) procrank
	
install:
	mv procrank ../bin/ -f

clean:
	rm -f *.o procrank


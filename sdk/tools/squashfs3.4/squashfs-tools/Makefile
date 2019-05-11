INSTALL_DIR = ../../../../scripts/
ZLIB_DIR=./zlib-install

INCLUDEDIR = .
ZLIB_INC=$(ZLIB_DIR)/include
ZLIB_LIB=$(ZLIB_DIR)/lib

CFLAGS := -I$(INCLUDEDIR) -I$(ZLIB_INC) -L$(ZLIB_LIB) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -O2

all: mksquashfs unsquashfs

mksquashfs: mksquashfs.o read_fs.o sort.o
	$(CC) mksquashfs.o read_fs.o sort.o -L$(ZLIB_LIB) -lz -lpthread -lm -o $@

mksquashfs.o: mksquashfs.c squashfs_fs.h mksquashfs.h global.h sort.h

read_fs.o: read_fs.c squashfs_fs.h read_fs.h global.h

sort.o: sort.c squashfs_fs.h global.h sort.h

unsquashfs: unsquashfs.o
	$(CC) unsquashfs.o -L$(ZLIB_LIB) -lz -lpthread -lm -o $@

unsquashfs.o: unsquashfs.c squashfs_fs.h read_fs.h global.h

clean:
	-rm -f *.o mksquashfs unsquashfs

install: mksquashfs unsquashfs
	cp mksquashfs unsquashfs $(INSTALL_DIR)

#ifndef SWF_GZIO_H
#define SWF_GZIO_H

#include "zlib.h"

#define MODE_RES_FILE			0x01
#define MODE_COMPRESS			0x00
#define MODE_GZIP_HEADER		0x10
#define MODE_NO_COMP_NO_BUF		0x20
#define MODE_NO_COMP_WITH_BUF	0x30

#define GET_ZLIB_MODE(mode) (mode & 0xF0)
#define GET_FILE_MODE(mode) (mode & 0x0F)


#define ORDER_MSB 0
#define ORDER_LSB 1

gzFile gzopen (const char *path, int mode, PFOPS fops, int size);

#define gzopen(path,mode,fops) \
	gzopenex(path,(mode)|MODE_GZIP_HEADER,fops,0)

gzFile gzopenex (const char *path, int mode, PFOPS fops, int size);

int gzread (gzFile file, voidp buf, unsigned int len); 

int gzrewind (gzFile file);

z_off_t gzseek (gzFile file, z_off_t offset, int whence);

z_off_t gztell (gzFile file);

int gzclose(gzFile file);

int gzreadbits(gzFile file, unsigned n, int lsb);

#endif

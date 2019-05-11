//#include "zmem.h"
#include "zlib.h"
//#include "amTypedef.h"

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */
#define EOF (-1) /* end of file */
//#define EOF ('\0') /* end of file */

/* ===========================================================================
   Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be large enough to hold the
   entire uncompressed data. (The size of the uncompressed data must have
   been saved previously by the compressor and transmitted to the decompressor
   by some mechanism outside the scope of this compression library.)
   Upon exit, destLen is the actual size of the compressed buffer.
     This function can be used to decompress a whole file at once if the
   input file is mmap'ed.

     mem_ungzip returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted.
*/
#define INT32S int
#define INT8S	char

INT32S mem_ungzip(unsigned char *dest, INT32S *destLen, const unsigned char *source, INT32S sourceLen)
{
    z_stream stream;
    INT32S err;
    const unsigned char *s = source;
    INT32S method;
    INT32S flags;
    INT32S len;
    INT8S c;

	/* Check the rest of the gzip header */
	s += 2;
    method = *s++;
    flags = *s++;
    if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
        return -1;
    }
	
    /* Discard time, xflags and OS code: */
    s += 6;
	
    if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
        len  =  *s++;
        len += ((uInt)*s++)<<8;
        /* len is garbage if EOF but the loop below will quit anyway */
        while (len-- != 0 && *s++ != EOF) ;
     // 	while (len-- != 0 )
	//		*s++;
    }
    if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
        while ((c = *s++) != 0 && c != EOF) ;
    }

    if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
        while ((c = *s++) != 0 && c != EOF) ;
    }
    if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
        for (len = 0; len < 2; len++) s++;
    }
//	len = *(unsigned short *)(s+26);
//	len += *(unsigned short *)(s+28);
//	len += 30;
	sourceLen -= (s - source) + 8;
	source = s;
//	sourceLen = *(uInt *)(s+18);
//	source = s+len;
	
    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    //err = inflateInit(&stream);
	err = inflateInit2(&stream,-15);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0)){				
            return Z_DATA_ERROR;
        }		
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);	
    return err;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ez_avutil.h"
#include "ez_avio.h"

static void fill_buffer(ByteIOContext *s)
{
    unsigned char *dst= s->buf_end - s->buffer < s->buffer_size ? s->buf_ptr : s->buffer;
    int len= s->buffer_size - (dst - s->buffer);

    //assert(s->buf_ptr == s->buf_end);

    /* no need to do anything if EOF already reached */
    if (s->eof_reached)
        return;

    if(s->read_packet)
        len = s->read_packet(s->opaque, dst, len);
    else
        len = 0;

    if (len <= 0) {
        /* do not modify buffer if EOF reached so that a seek back can
           be done without rereading data */
        s->eof_reached = 1;
        if(len<0)
            s->error= len;
    } else {
        s->pos += len;
        s->buf_ptr = dst;
        s->buf_end = dst + len;
    }
}

long avio_read(ByteIOContext *s, unsigned char *buf, long size)
{
    int len, size1;

    size1 = size;
    while (size > 0) {
        len = s->buf_end - s->buf_ptr;
        if (len > size)
            len = size;
        if (len == 0) {
            if(size > s->buffer_size){
                if(s->read_packet)
                    len = s->read_packet(s->opaque, buf, size);
                if (len <= 0) {
                    /* do not modify buffer if EOF reached so that a seek back can
                    be done without rereading data */
                    s->eof_reached = 1;
                    if(len<0)
                        s->error= len;
                    break;
                } else {
                    s->pos += len;
                    size -= len;
                    buf += len;
                    s->buf_ptr = s->buffer;
                    s->buf_end = s->buffer/* + len*/;
                }
            }else{
                len = s->read_packet(s->opaque, s->buffer, s->buffer_size);
                if(len > 0){
                    s->pos += len;
                    s->buf_ptr = s->buffer;
                    s->buf_end = s->buffer + len;
                }else{
                    s->eof_reached = 1;
                    if(len<0)
                        s->error= len;
                    break;
                }
            }
        } else {
            memcpy(buf, s->buf_ptr, len);
            buf += len;
            s->buf_ptr += len;
            size -= len;
        }
    }
    return size1 - size;
}

/*
    Just use for ts miracast read packet. Each time read 188Bytes. 
    Not copy to buf, just return buffer cache ptr.
    return: read size
*/
long avio_read_ptr(ByteIOContext *s, unsigned char **buf, long size)
{
    int len, size1;

    if(size>s->buffer_size){
        return -1;
    }

    len = s->buf_end - s->buf_ptr;
    if(len<size){
        if(len>0)
            memcpy(s->buffer, s->buf_ptr, len);
        s->buf_ptr = s->buffer;
        s->buf_end = s->buf_ptr+len;
        len = s->read_packet(s->opaque, s->buf_end, s->buffer_size-len);
        if(len > 0){
            s->pos += len;
            s->buf_end += len;
        }else{
            s->eof_reached = 1;
            if(len<0)
                s->error= len;
            return -1;
        }
    }
    
    len = s->buf_end - s->buf_ptr;
    *buf = s->buf_ptr;
    if(len>=size){
        size1 = size;
    }else{
        size1 = len;
    }
    
    s->buf_ptr += size1;
    
    return size1;
}


unsigned long avio_r8(ByteIOContext *s)
{
	if(s->buf_ptr >= s->buf_end)
		fill_buffer(s);

	if (s->buf_ptr < s->buf_end)
		return *s->buf_ptr++;
	else
		return 0;
}

unsigned long avio_rl16(ByteIOContext *s)
{
	long val;
	val = avio_r8(s);
	val |= avio_r8(s) << 8;
	return val;
}

unsigned long avio_rl24(ByteIOContext *s)
{
	long val;
	val = avio_rl16(s);
	val |= avio_r8(s) << 16;
	return val;
}

unsigned long avio_rl32(ByteIOContext *s)
{
	long val;
	val = avio_rl16(s);
	val |= avio_rl16(s) << 16;
	return val;
}

av_i64 avio_rl64(ByteIOContext *s)
{
	av_i64 val;
	val = (av_u64)avio_rl32(s);
	val |= (av_u64)avio_rl32(s) << 32;
	return val;
}

unsigned long avio_rb16(ByteIOContext *s)
{
	long val;
	val = avio_r8(s) << 8;
	val |= avio_r8(s);
	return val;
}

unsigned long avio_rb24(ByteIOContext *s)
{
	long val;
	val = avio_rb16(s) << 8;
	val |= avio_r8(s);
	return val;
}

unsigned long avio_rb32(ByteIOContext *s)
{
	long val;
	val = avio_rb16(s) << 16;
	val |= avio_rb16(s);
	return val;
}

av_i64 avio_rb64(ByteIOContext *s)
{
	av_i64 val;
	val = (av_u64)avio_rb32(s) << 32;
	val |= (av_u64)avio_rb32(s);
	return val;
}

static int init_put_byte(ByteIOContext *s,
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  long (*read_packet)(void *opaque, unsigned char *buf, long buf_size),
                  long (*write_packet)(void *opaque, unsigned char *buf, long buf_size),
				  long (*seek_set)(void *opaque, av_i64 offset),
				  long (*seek_end)(void *opaque, av_i64 offset),
				  av_i64 (*get_pos)(void *opaque),
				  int (*close)(void *opaque),
				  av_i64 (*get_length)(void *opaque))
{
    s->buffer = buffer;
    s->buffer_size = buffer_size;
    s->buf_ptr = buffer;
    s->buf_end = buffer;
    s->opaque = opaque;
    s->write_packet = write_packet;
    s->read_packet = read_packet;
    s->seek_set = seek_set;
    s->seek_end =seek_end;
    s->get_pos = get_pos;
    s->close = close;
    s->get_length= get_length;
    s->pos = 0;
    s->must_flush = 0;
    s->eof_reached = 0;
    s->error = 0;
    s->is_streamed = 0;
    s->seekable = 1;
    s->max_packet_size = 0;
    s->update_checksum= NULL;

    return 0;
}


int avio_open(ByteIOContext **s, char * const name, av_file_iocontext_s *f_io)
{
     void *h=NULL;
     unsigned char *buffer;

    h = f_io->open(name, "rb");
    if(!h){
        return AVERROR(ENOFILE);
    }
    
    buffer = (unsigned char *)av_malloc(IO_BUFFER_SIZE);
    if (!buffer){
        f_io->close(h);	 	
        return AVERROR(ENOMEM);
    }
     
    *s = (ByteIOContext *)av_mallocz(sizeof(ByteIOContext));
    if(!*s) {
        av_free(buffer);
        f_io->close(h);			
        return AVERROR(ENOMEM);
    }
     if (init_put_byte(*s, buffer, IO_BUFFER_SIZE,
     				  0, h,
     				  f_io->read, f_io->write, f_io->seek_set, f_io->seek_end, f_io->tell,
					  f_io->close,f_io->get_length) < 0) {

     	av_free(buffer);
     	av_freep(s);
	f_io->close(h);
     	return AVERROR(EIO);
     }

     return 0;
}

int avio_close(ByteIOContext *s)
{
    void *h = s->opaque;
    int ret;

    ret = s->close(h);
    av_free(s->buffer);
    av_free(s);
    return ret;
}

void *avio_fileno(ByteIOContext *s)
{
    return s->opaque;
}

static
av_i64 url_fseek_set(ByteIOContext *s, av_i64 offset)
{
    av_i64 offset1;
    av_i64 pos;
	
    pos = s->pos - (s->write_flag ? 0 : (s->buf_end - s->buffer));
    offset1 = offset - pos;
	
    if (offset1 >= 0 && offset1 < (s->buf_end - s->buffer)) {
    	/* can do the seek inside the buffer */
    	s->buf_ptr = s->buffer + offset1;
    }else {
    	if(s->seek_set(s->opaque, offset) < 0)
    		return -1;
    	if (!s->write_flag)
    		s->buf_end = s->buffer;
    	s->buf_ptr = s->buffer;
    	s->pos = offset;
    }
    s->eof_reached = 0;
    return offset;
}

static 
av_i64 url_fseek_cur(ByteIOContext *s, av_i64 offset)
{
    av_i64 offset1;
    av_i64 pos;

    pos = s->pos - (s->write_flag ? 0 : (s->buf_end - s->buffer));
    offset1 = pos + (s->buf_ptr - s->buffer);
    if (offset == 0)
        return offset1;
    offset += offset1;
    offset1 = offset - pos;

    if (offset1 >= 0 && offset1 < (s->buf_end - s->buffer)) {
         /* can do the seek inside the buffer */
        s->buf_ptr = s->buffer + offset1;
    }else {
        if(s->seek_set(s->opaque, offset) < 0)
            return -1;
        if (!s->write_flag)
            s->buf_end = s->buffer;
        s->buf_ptr = s->buffer;
        s->pos = offset;
    }
    s->eof_reached = 0;
    return offset;
}

av_i64 avio_seek(ByteIOContext *s, av_i64 offset, int flag)
{
    if(SEEK_CUR==flag){
        return url_fseek_cur(s, offset);
    }else{
        return url_fseek_set(s, offset);
    }    
}

void avio_skip(ByteIOContext *s, av_i64 offset)
{
	url_fseek_cur(s, offset);
}

av_i64 avio_tell(ByteIOContext *s)
{
	return url_fseek_cur(s, 0);
}

av_i64 avio_size(ByteIOContext *s)
{
	av_i64 size, pos;

	if (s->get_length)
	{
		size = s->get_length(s->opaque);
	}
	else
	{
		pos = s->get_pos(s->opaque);
		s->seek_end(s->opaque, 0);
		size = s->get_pos(s->opaque);
		s->seek_set(s->opaque, pos);
	}
	return size;
}

long avio_eof(ByteIOContext *s)
{
	return s->eof_reached;
}

#ifndef _EZ_AVIO_H_
#define _EZ_AVIO_H_


#define IO_BUFFER_SIZE (16*1024)


typedef struct
{
	void *(*open)(char *path, char *mode);
	long (*read)(void *fp, unsigned char *buf, long buf_size);
	long (*write)(void *fp, unsigned char *buf, long buf_size);
	long (*seek_set)(void *fp, av_i64 offset);
	long (*seek_end)(void *fp, av_i64 offset);
	av_i64 (*tell)(void *fp);
	int (*close)(void *fp);
	av_i64 (*get_length)(void *fp);
	int (*get_seekable)(void *fp);
	int (*set_stop)(void *fp);
}av_file_iocontext_s;


typedef struct _ByteIOContext_
{
    unsigned char *buffer;
    long buffer_size;
    unsigned char *buf_ptr;
    unsigned char *buf_end;
    void *opaque;
    long (*read_packet)(void *opaque, unsigned char *buf, long buf_size);
    long (*write_packet)(void *opaque, unsigned char *buf, long buf_size);
    long (*seek_set)(void *opaque, av_i64 offset);
    long (*seek_end)(void *opaque, av_i64 offset);
    av_i64 (*get_pos)(void *opaque);
    av_i64 (*get_length)(void *opaque);
    av_i64 pos;		/* position in the file of the current buffer */
    int must_flush; /**< true if the next seek should flush */    
    int eof_reached;	/* true if eof reached */
    int write_flag;		/**< true if open for writing */
    int is_streamed;
    int max_packet_size;
    unsigned long checksum;    
    unsigned char *checksum_ptr;    
    unsigned long (*update_checksum)(unsigned long checksum, const unsigned char *buf, unsigned int size);    
    int error;			/* contains the error code or 0 if no error happened */
    int (*read_pause)(void *opaque, int pause);   
    av_i64 (*read_seek)(void *opaque, int stream_index, av_i64 timestamp, int flags);
    int (*close)(void *opaque);
    /**
    * A combination of AVIO_SEEKABLE_ flags or 0 when the stream is not seekable.
    */
    int seekable;
}ByteIOContext;



long avio_read(ByteIOContext *s, unsigned char *buf, long size);
long avio_read_ptr(ByteIOContext *s, unsigned char **buf, long size);
unsigned long avio_r8(ByteIOContext *s);
unsigned long avio_rl16(ByteIOContext *s);
unsigned long avio_rl24(ByteIOContext *s);
unsigned long avio_rl32(ByteIOContext *s);
av_i64 avio_rl64(ByteIOContext *s);
unsigned long avio_rb16(ByteIOContext *s);
unsigned long avio_rb24(ByteIOContext *s);
unsigned long avio_rb32(ByteIOContext *s);
av_i64 avio_rb64(ByteIOContext *s);


int avio_open(ByteIOContext **s, char * const name, av_file_iocontext_s *f_io);
int avio_close(ByteIOContext *s);
void *avio_fileno(ByteIOContext *s);
av_i64 avio_seek(ByteIOContext *s, av_i64 offset, int flag);
void avio_skip(ByteIOContext *s, av_i64 offset);
av_i64 avio_tell(ByteIOContext *s);
long avio_eof(ByteIOContext *s);


#endif//_AVIO_H_



